#include "tp_utils/Garbage.h"
#include "tp_utils/MutexUtils.h"

#include <thread>
#include <queue>

namespace tp_utils
{

namespace
{
//##################################################################################################
struct Garbage
{
  //################################################################################################
  Garbage()
  {
    for(size_t i=0; i<4; i++)
    threads.push_back(new std::thread([&]
    {
      TP_MUTEX_LOCKER(mutex);
      while(!finish || !queue.empty())
      {
        if(queue.empty())
          waitCondition.wait(mutex);
        else
        {
          auto closure = queue.front();
          queue.pop();
          {
            TP_MUTEX_UNLOCKER(mutex);
            closure();
          }
        }
      }
    }));
  }

  //################################################################################################
  ~Garbage()
  {
    mutex.locked([&]{finish = true;});
    waitCondition.wakeAll();
    while(!threads.empty())
    {
      auto thread = tpTakeLast(threads);
      thread->join();
      delete thread;
    }
  }

  //################################################################################################
  void garbage(const std::function<void()>& closure)
  {
    mutex.locked([&]{queue.push(closure);});
    waitCondition.wakeOne();
  }

  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
  bool finish{false};
  std::vector<std::thread*> threads;
  std::queue<std::function<void()>> queue;
};
}

//##################################################################################################
void garbage(const std::function<void()>& closure)
{
#ifdef TP_NO_THREADS
  closure();
#else
  static Garbage garbage;
  garbage.garbage(closure);
#endif
}

}
