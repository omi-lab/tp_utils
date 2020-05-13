#include "tp_utils/SignalHandler.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/RefCount.h"

#include <csignal>

namespace tp_utils
{

#if defined TP_WIN32
using SignalHandlerT = _crt_signal_t;
#elif defined TP_EMSCRIPTEN
using SignalHandlerT = sighandler_t;
#elif defined TP_OSX
using SignalHandlerT = void (*)(int);
#elif defined TP_IOS
using SignalHandlerT = void (*)(int);
#else
using SignalHandlerT = __sighandler_t;
#endif

//##################################################################################################
struct SignalHandler::Private
{
  TP_REF_COUNT_OBJECTS("tp_utils::SignalHandler::Private");
  TP_NONCOPYABLE(Private);

  SignalHandlerT signt;
  SignalHandlerT sigterm;

  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;

  static Private* instance;

  //################################################################################################
  Private()
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
    TP_UNUSED(sig);
    instance->waitCondition.wakeAll();
  }
};

//##################################################################################################
SignalHandler::Private* SignalHandler::Private::instance{nullptr};

//##################################################################################################
SignalHandler::SignalHandler():
  d(new Private)
{
  d->signt = std::signal(SIGINT, Private::exitWake);
  d->sigterm = std::signal(SIGTERM, Private::exitWake);
}

//##################################################################################################
SignalHandler::~SignalHandler()
{
  std::signal(SIGINT, d->signt);
  std::signal(SIGTERM, d->sigterm);
  delete d;
}

//##################################################################################################
void SignalHandler::waitCtrlC()
{
  TP_MUTEX_LOCKER(d->mutex);
  d->waitCondition.wait(TPMc d->mutex);
}

}
