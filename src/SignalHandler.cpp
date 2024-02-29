#include "tp_utils/SignalHandler.h"
#include "tp_utils/StackTrace.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/RefCount.h"

#include <csignal>
#include <iostream>

#ifdef TP_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <new.h>
#include <signal.h>
#include <exception>
#include <sys/stat.h>
#include <Psapi.h>
#include <rtcapi.h>
#include <shellapi.h>
#include <DbgHelp.h>
#endif

namespace tp_utils
{

#if defined TP_WIN32_MINGW
using SignalHandlerT = void (*)(int);
#elif defined TP_WIN32
using SignalHandlerT = _crt_signal_t;
#elif defined TP_EMSCRIPTEN
using SignalHandlerT = sighandler_t;
#else
using SignalHandlerT = void (*)(int);
#endif

//##################################################################################################
struct SignalHandler::Private
{
  TP_REF_COUNT_OBJECTS("tp_utils::SignalHandler::Private");
  TP_NONCOPYABLE(Private);

  bool exitOnInt;
  void (*handler)(int);

  SignalHandlerT sigint;
  SignalHandlerT sigterm;

  SignalHandlerT sigabrt;
  SignalHandlerT sigfpe;
  SignalHandlerT sigill;
  SignalHandlerT sigsegv;

  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
  bool shouldExit{false};

  static Private* instance;

  //################################################################################################
  Private(bool exitOnInt_, void (*handler_)(int)):
    exitOnInt(exitOnInt_),
    handler(handler_)
  {
    instance = this;

    sigint  = std::signal(SIGINT , exitWake);
    sigterm = std::signal(SIGTERM, exitWake);

#ifndef TP_DEBUG
#  ifdef TP_WIN32_MSVC
    setWindowsHandlers();
#  else
    sigabrt = std::signal(SIGABRT, exitNow);
    sigfpe  = std::signal(SIGFPE , exitNow);
    sigill  = std::signal(SIGILL , exitNow);
    sigsegv = std::signal(SIGSEGV, exitNow);
#  endif
#endif
  }

  //################################################################################################
  ~Private()
  {
    instance = nullptr;
    std::signal(SIGINT , sigint );
    std::signal(SIGTERM, sigterm);

#ifndef TP_WIN32_MSVC
    std::signal(SIGABRT, sigabrt);
    std::signal(SIGFPE , sigfpe );
    std::signal(SIGILL , sigill );
    std::signal(SIGSEGV, sigsegv);
#endif
  }

#ifdef TP_WIN32_MSVC
  //################################################################################################
  // Lifted from:
  // https://www.codeproject.com/Articles/207464/Exception-Handling-in-Visual-Cplusplus
  void setWindowsHandlers()
  {
    SetUnhandledExceptionFilter(sehHandler);
    _set_purecall_handler(pureCallHandler);
    _set_new_handler(newHandler);
    _set_invalid_parameter_handler(invalidParameterHandler);
    _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);

    set_terminate(terminateHandler);
    set_unexpected(unexpectedHandler);
  }

  //################################################################################################
  // Install top-level SEH handler
  // Structured exception handler
  [[noreturn]]static LONG WINAPI sehHandler(PEXCEPTION_POINTERS pExceptionPtrs)
  {
    std::cout << "sehHandler()" << std::endl;
    saveCrashReportAndExit(pExceptionPtrs);
  }

  //################################################################################################
  // Catch pure virtual function calls.
  // Because there is one _purecall_handler for the whole process,
  // calling this function immediately impacts all threads. The last
  // caller on any thread sets the handler.
  // http://msdn.microsoft.com/en-us/library/t296ys27.aspx
  // CRT Pure virtual method call handler
  [[noreturn]]static void __cdecl pureCallHandler()
  {
    std::cout << "pureCallHandler()" << std::endl;
    saveCrashReportAndExit();
  }

  //################################################################################################
  // Catch new operator memory allocation exceptions
  // CRT new operator fault handler
  [[noreturn]]static int __cdecl newHandler(size_t)
  {
    std::cout << "newHandler()" << std::endl;
    // 'new' operator memory allocation exception
    saveCrashReportAndExit();
  }

  //################################################################################################
  // Catch invalid parameter exceptions.
  // CRT invalid parameter handler
  [[noreturn]]static void __cdecl invalidParameterHandler(const wchar_t* expression,
                                              const wchar_t* function,
                                              const wchar_t* file,
                                              unsigned int line,
                                              uintptr_t pReserved)
  {
    std::cout << "invalidParameterHandler()" << std::endl;
    std::cout << "expression: " << expression << std::endl;
    std::cout << "function  : " << function << std::endl;
    std::cout << "file      : " << file << std::endl;
    std::cout << "line      : " << line << std::endl;

    // Invalid parameter exception
    TP_UNUSED(pReserved);
    saveCrashReportAndExit();
  }

  //################################################################################################
  // Catch terminate() calls.
  // In a multithreaded environment, terminate functions are maintained
  // separately for each thread. Each new thread needs to install its own
  // terminate function. Thus, each thread is in charge of its own termination handling.
  // http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
  // CRT terminate() call handler
  [[noreturn]]static void __cdecl terminateHandler()
  {
    std::cout << "terminateHandler()" << std::endl;
    // Abnormal program termination (terminate() function was called)
    saveCrashReportAndExit();
  }

  //################################################################################################
  // Catch unexpected() calls.
  // In a multithreaded environment, unexpected functions are maintained
  // separately for each thread. Each new thread needs to install its own
  // unexpected function. Thus, each thread is in charge of its own unexpected handling.
  // http://msdn.microsoft.com/en-us/library/h46t5b69.aspx
  // CRT unexpected() call handler
  [[noreturn]]static void __cdecl unexpectedHandler()
  {
    std::cout << "unexpectedHandler()" << std::endl;
    // Unexpected error (unexpected() function was called)
    saveCrashReportAndExit();
  }
#endif

  //################################################################################################
  static void exitWake(int sig)
  {
    std::cout << "\nSignal caught: " << sig << std::endl;

    if(instance->handler)
    {
      instance->handler(sig);
      return;
    }

    if(instance->exitOnInt)
      saveCrashReportAndExit();

    instance->shouldExit = true;
    instance->waitCondition.wakeAll();
  }

  //################################################################################################
  [[noreturn]]static void exitNow(int sig)
  {
    std::cout << "\nFatal signal caught: " << sig << std::endl;
    saveCrashReportAndExit();
  }
};

//##################################################################################################
SignalHandler::Private* SignalHandler::Private::instance{nullptr};

//##################################################################################################
SignalHandler::SignalHandler(bool exitOnInt, void (*handler)(int)):
  d(new Private(exitOnInt, handler))
{

}

//##################################################################################################
SignalHandler::~SignalHandler()
{
  delete d;
}

//##################################################################################################
void SignalHandler::setExitOnInt(bool exitOnInt)
{
  d->exitOnInt = exitOnInt;
}

//##################################################################################################
void SignalHandler::waitCtrlC()
{
  TPMutexLocker lock(d->mutex);
  d->waitCondition.wait(TPMc lock);
}

//##################################################################################################
bool SignalHandler::shouldExit() const
{
  return d->shouldExit;
}

}
