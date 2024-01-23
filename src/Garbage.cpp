#include "tp_utils/Garbage.h"
#include "tp_utils/MutexUtils.h"

#include "lib_platform/SetThreadName.h"

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
    {
      threads.push_back(new std::thread([&]
      {
        lib_platform::setThreadName("Garbage");
        TP_MUTEX_LOCKER(mutex);
        while(!finish || !queue.empty())
        {
          if(queue.empty())
            waitCondition.wait(TPMc mutex);
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
  }

  //################################################################################################
  ~Garbage()
  {
    mutex.locked(TPMc [&]{finish = true;});
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
    mutex.locked(TPMc [&]{queue.push(closure);});
    waitCondition.wakeOne();
  }

  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
  bool finish{false};
  std::vector<std::thread*> threads;
  std::queue<std::function<void()>> queue;
};
}

namespace {
  static std::unique_ptr<Garbage> garbage_aux;
}

//##################################################################################################
void initGarbage()
{
  garbage_aux.reset(new Garbage);
  tp_utils::garbage([]{});
}

//##################################################################################################
void garbage(const std::function<void()>& closure)
{
#ifdef TP_NO_THREADS
  closure();
#else
  if(garbage_aux)
    garbage_aux->garbage(closure);
  else
    closure();
#endif
}

GarbageLocker::GarbageLocker()
{
  initGarbage();
}

GarbageLocker::~GarbageLocker()
{
  garbage_aux.reset();
}

}
