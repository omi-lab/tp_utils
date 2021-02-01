#include "tp_utils/SignalHandler.h"
#include "tp_utils/StackTrace.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/FileUtils.h"
#include "tp_utils/RefCount.h"

#include <csignal>
#include <iostream>

namespace tp_utils
{

#if defined TP_WIN32
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

  SignalHandlerT sigint;
  SignalHandlerT sigterm;

  SignalHandlerT sigabrt;
  SignalHandlerT sigfpe;
  SignalHandlerT sigill;
  SignalHandlerT sigsegv;

  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;

  static Private* instance;

  //################################################################################################
  Private(bool exitOnInt_):
    exitOnInt(exitOnInt_)
  {
    instance = this;
  }

  //################################################################################################
  ~Private()
  {
    instance = nullptr;
  }

  //################################################################################################
  static void exitWake(int sig)
  {
    std::cout << "Signal caught: " << sig << '\n';
    if(instance->exitOnInt)
    {
      printStackTrace();
      tp_utils::writeTextFile("crash.txt", formatStackTrace());
      exit(sig);
    }
    instance->waitCondition.wakeAll();
  }

  //################################################################################################
  static void exitNow(int sig)
  {
    std::cout << "Fatal signal caught: " << sig << '\n';
    printStackTrace();
    exit(sig);
  }
};

//##################################################################################################
SignalHandler::Private* SignalHandler::Private::instance{nullptr};

//##################################################################################################
SignalHandler::SignalHandler(bool exitOnInt):
  d(new Private(exitOnInt))
{
  d->sigint  = std::signal(SIGINT , Private::exitWake);
  d->sigterm = std::signal(SIGTERM, Private::exitWake);

  d->sigabrt = std::signal(SIGABRT, Private::exitNow);
  d->sigfpe  = std::signal(SIGFPE , Private::exitNow);
  d->sigill  = std::signal(SIGILL , Private::exitNow);
  d->sigsegv = std::signal(SIGSEGV, Private::exitNow);
}

//##################################################################################################
SignalHandler::~SignalHandler()
{
  std::signal(SIGINT , d->sigint );
  std::signal(SIGTERM, d->sigterm);

  std::signal(SIGABRT, d->sigabrt);
  std::signal(SIGFPE , d->sigfpe );
  std::signal(SIGILL , d->sigill );
  std::signal(SIGSEGV, d->sigsegv);

  delete d;
}

//##################################################################################################
void SignalHandler::waitCtrlC()
{
  TP_MUTEX_LOCKER(d->mutex);
  d->waitCondition.wait(TPMc d->mutex);
}

}
