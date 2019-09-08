#include "tp_utils/SignalHandler.h"
#include "tp_utils/MutexUtils.h"

#include <csignal>

namespace tp_utils
{

//##################################################################################################
struct SignalHandler::Private
{
  __sighandler_t signt;
  __sighandler_t sigterm;

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
