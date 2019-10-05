#include "tp_utils/SignalHandler.h"
#include "tp_utils/MutexUtils.h"

#include <csignal>

namespace tp_utils
{

#if defined TP_WIN32
using SignalHandlerT = _crt_signal_t;
#define
#elif defined TP_EMSCRIPTEN
using SignalHandlerT = sighandler_t;
#else
using SignalHandlerT = __sighandler_t;
#endif

//##################################################################################################
struct SignalHandler::Private
{
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
