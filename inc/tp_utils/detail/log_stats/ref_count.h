#ifdef TP_REF_COUNT

#include "tp_utils/detail/log_stats/impl.h"
#include "tp_utils/RefCount.h"

namespace tp_utils
{

namespace detail
{

//##################################################################################################
template<class T = void>
struct TP_UTILS_EXPORT _SaveRefCountTimer : public LogStatsTimer
{
  _SaveRefCountTimer(const std::string& path, int64_t intervalMS):
    LogStatsTimer(path, intervalMS, RefCount::takeResults)
  {

  }
};

//##################################################################################################
using SaveRefCountTimer = _SaveRefCountTimer<void>;
}

//##################################################################################################
inline void addRefCountProducer(detail::KeyValueLogStatsTimer& keyValueStatsTimer)
{
  keyValueStatsTimer.addProducer("RefCount ", [=]
  {
    return RefCount::keyValueResults();
  });
}

}

#endif
