#ifdef TP_ENABLE_MUTEX_TIME

#include "tp_utils/detail/log_stats/impl.h"

namespace tp_utils
{

namespace detail
{

//##################################################################################################
template<class T = void>
struct TP_UTILS_SHARED_EXPORT _SaveLockStatsTimer : public LogStatsTimer
{
  _SaveLockStatsTimer(const std::string& path, int64_t intervalMS):
    LogStatsTimer(path, intervalMS, LockStats::takeResults)
  {

  }
};
using SaveLockStatsTimer = _SaveLockStatsTimer<void>;

}

}

#endif
