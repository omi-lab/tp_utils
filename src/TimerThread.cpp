#include "tp_utils/TimerThread.h"
#include "tp_utils/MutexUtils.h"

#include <thread>

namespace tp_utils
{

//##################################################################################################
struct TimerThread::Private
{
  std::function<void()> callback;
  int64_t timeoutMS;

  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
  bool finish{false};

  std::thread thread;

  //################################################################################################
  Private(const std::function<void()>& callback_, int64_t timeoutMS_):
    callback(callback_),
    timeoutMS(timeoutMS_),
    thread([&]{run();})
  {

  }

  //################################################################################################
  ~Private()
  {
    {
      TP_MUTEX_LOCKER(mutex);
      finish = true;
      waitCondition.wakeAll();
    }

    thread.join();
  }

  //################################################################################################
  void run()
  {
    TP_MUTEX_LOCKER(mutex);
    while(!finish)
    {
      waitCondition.wait(TPMc mutex, timeoutMS);

      if(!finish)
      {
        TP_MUTEX_UNLOCKER(mutex);
        callback();
      }
    }
  }
};

//##################################################################################################
TimerThread::TimerThread(const std::function<void()>& callback, int64_t timeoutMS):
  d(new Private(callback, timeoutMS))
{

}

//##################################################################################################
TimerThread::~TimerThread()
{
  delete d;
}

}
