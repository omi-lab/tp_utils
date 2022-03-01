#ifndef tp_utils_Parallel_h
#define tp_utils_Parallel_h

#include "tp_utils/Globals.h"

//#define TP_NO_THREADS

#ifndef TP_NO_THREADS
#include "tp_utils/MutexUtils.h"

#include <thread>
#endif

namespace tp_utils
{

//##################################################################################################
template<typename T>
void parallel(T worker)
{
#ifdef TP_NO_THREADS
  auto locker = [&](auto closure){closure();};
  worker(locker);
#else
  TPMutex mutex{TPM};
  auto locker = [&](auto closure){mutex.locked(TPMc closure);};

  size_t threads = std::thread::hardware_concurrency();
  std::vector<std::thread> workers;
  workers.reserve(threads);
  for(size_t i=0; i<threads; i++)
    workers.emplace_back([&]{worker(locker);});

  for(auto& thread : workers)
    thread.join();
#endif
}

}

#endif
