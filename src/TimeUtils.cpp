#include "tp_utils/TimeUtils.h"

#include <chrono>

namespace tp_utils
{
//##################################################################################################
int64_t currentTime()
{
  return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

//##################################################################################################
int64_t currentTimeMS()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

//##################################################################################################
struct ElapsedTimer::Private
{
  std::chrono::steady_clock::time_point start;
};

//##################################################################################################
ElapsedTimer::ElapsedTimer():
  d(new Private())
{

}

//##################################################################################################
ElapsedTimer::~ElapsedTimer()
{
  delete d;
}

//##################################################################################################
void ElapsedTimer::start()
{
  d->start = std::chrono::steady_clock::now();
}

//##################################################################################################
int64_t ElapsedTimer::restart()
{
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - d->start).count();
  d->start = now;
  return elapsed;
}

//##################################################################################################
int64_t ElapsedTimer::elapsed()const
{
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now - d->start).count();
}

}
