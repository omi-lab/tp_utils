#ifndef BARRIERWITHCOUNTER_H
#define BARRIERWITHCOUNTER_H

#include <mutex>
#include <atomic>
#include <condition_variable>

namespace tp_utils {

class BarrierWithCounter
{
public:
  BarrierWithCounter()
  {
  }

  void lock()
  {
    ++count;
  }

  void release()
  {
    --count;
    cv.notify_all();
  }

  void wait()
  {
    std::unique_lock<std::mutex> lk(cv_m);
    cv.wait(lk, [&]{ return count == 0;} );
  }

  std::mutex cv_m; // mutex is open occasionally it can be used in threads
private:
  std::condition_variable cv;
  std::atomic_int count = 0;
};

}

#endif // BARRIERWITHCOUNTER_H
