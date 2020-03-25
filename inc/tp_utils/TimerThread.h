#ifndef tp_utils_TimerThread_h
#define tp_utils_TimerThread_h

#include "tp_utils/Globals.h"

namespace tp_utils
{

//##################################################################################################
class TP_UTILS_SHARED_EXPORT TimerThread
{
public:
  //################################################################################################
  TimerThread(const std::function<void()>& callback, int64_t timeoutMS);

  //################################################################################################
  virtual ~TimerThread();

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
