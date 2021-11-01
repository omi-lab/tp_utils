#ifdef TP_ENABLE_FUNCTION_TIME

#include "tp_utils/detail/log_stats/impl.h"
#include "tp_utils/TimeUtils.h"

namespace tp_utils
{

namespace detail
{
//##################################################################################################
template<class T = void>
struct TP_UTILS_SHARED_EXPORT _SaveFunctionTimeStatsTimer : public LogStatsTimer
{
  _SaveFunctionTimeStatsTimer(const std::string& path, int64_t intervalMS):
    LogStatsTimer(path, intervalMS, FunctionTimeStats::takeResults)
  {

  }
};
using SaveFunctionTimeStatsTimer = _SaveFunctionTimeStatsTimer<void>;

}

//##################################################################################################
inline void addFunctionTimeProducer(detail::KeyValueLogStatsTimer& keyValueStatsTimer)
{
  keyValueStatsTimer.addProducer("FunctionTime ", [=]
  {
    return FunctionTimeStats::keyValueResults();
  });
}


}
#endif
