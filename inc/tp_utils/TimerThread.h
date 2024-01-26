#ifndef tp_utils_TimerThread_h
#define tp_utils_TimerThread_h

#include "tp_utils/Globals.h"

namespace tp_utils
{

//##################################################################################################
class TP_UTILS_EXPORT TimerThread
{
  TP_NONCOPYABLE(TimerThread);
public:
  //################################################################################################
  TimerThread(const std::function<void()>& callback, int64_t timeoutMS, const std::string& threadName);

  //################################################################################################
  virtual ~TimerThread();

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
