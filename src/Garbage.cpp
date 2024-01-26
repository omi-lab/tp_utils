#include "tp_utils/Garbage.h"
#include "tp_utils/MutexUtils.h"

#include "lib_platform/SetThreadName.h"

#include <thread>
#include <queue>

namespace tp_utils
{
static Garbage* instance{nullptr};

//##################################################################################################
struct Garbage::Private
{
  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
  bool finish{false};
  std::vector<std::thread*> threads;
  std::queue<std::function<void()>> queue;
};

//##################################################################################################
Garbage::Garbage():
  d(new Private)
{
  if(!instance)
    instance = this;

  for(size_t i=0; i<4; i++)
  {
    d->threads.push_back(new std::thread([&]
    {
      lib_platform::setThreadName("Garbage");
      TP_MUTEX_LOCKER(d->mutex);
      while(!d->finish || !d->queue.empty())
      {
        if(d->queue.empty())
          d->waitCondition.wait(TPMc d->mutex);
        else
        {
          auto closure = d->queue.front();
          d->queue.pop();
          {
            TP_MUTEX_UNLOCKER(d->mutex);
            closure();
          }
        }
      }
    }));
  }
}

//##################################################################################################
Garbage::~Garbage()
{
  instance = nullptr;

  d->mutex.locked(TPMc [&]{d->finish = true;});
  d->waitCondition.wakeAll();
  while(!d->threads.empty())
  {
    auto thread = tpTakeLast(d->threads);
    thread->join();
    delete thread;
  }

  delete d;
}

//##################################################################################################
void Garbage::garbage(const std::function<void()>& closure)
{
  d->mutex.locked(TPMc [&]{d->queue.push(closure);});
  d->waitCondition.wakeOne();
}

//##################################################################################################
void garbage(const std::function<void()>& closure)
{
#ifdef TP_NO_THREADS
  closure();
#else
  if(instance)
    instance->garbage(closure);
  else
    closure();
#endif
}

}
