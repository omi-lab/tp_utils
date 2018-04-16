#ifndef tp_utils_TimeUtils_h
#define tp_utils_TimeUtils_h

#include "tp_utils/Globals.h"

namespace tp_utils
{

//##################################################################################################
//! Returns the current time as a 64 bit Unix time stamp
/*!
\return the current time as a 64 bit Unix time stamp
*/
int64_t TP_UTILS_SHARED_EXPORT currentTime();

//##################################################################################################
//! Returns the current time as ms since SinceEpoch
/*!
\return the current time as ms since since the epoch
*/
int64_t TP_UTILS_SHARED_EXPORT currentTimeMS();

//##################################################################################################
class ElapsedTimer
{
public:
  //################################################################################################
  ElapsedTimer();

  //################################################################################################
  ~ElapsedTimer();

  //################################################################################################
  void start();

  //################################################################################################
  int64_t restart();

  //################################################################################################
  int64_t elapsed()const;

private:
  struct Private;
  Private* d;
};

}

#endif
