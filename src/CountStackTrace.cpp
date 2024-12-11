#include "tp_utils/CountStackTrace.h"
#include "tp_utils/StackTrace.h" // IWYU pragma: keep
#include "tp_utils/MutexUtils.h" // IWYU pragma: keep
#include "tp_utils/DebugUtils.h" // IWYU pragma: keep

#include <map>

namespace tp_utils
{
#ifdef TP_ENABLE_COUNT_STACK_TRACE

namespace
{
//##################################################################################################
struct CountStackTraceStats
{
  TPMutex mutex{TPM};
  std::unordered_map<StackTrace, size_t> counts;
};

//##################################################################################################
CountStackTraceStats& countStackTraceStats()
{
  static CountStackTraceStats countStackTraceStats;
  return countStackTraceStats;
}
}

//##################################################################################################
void CountStackTrace::init()
{
  auto& s = countStackTraceStats();
  TP_MUTEX_LOCKER(s.mutex);
}

//##################################################################################################
void CountStackTrace::count()
{
  auto& s = countStackTraceStats();
  TP_MUTEX_LOCKER(s.mutex);
  s.counts[StackTrace()]++;
}

//##################################################################################################
void CountStackTrace::print()
{
  std::vector<std::pair<std::string, size_t>> counts;

  {
    auto& s = countStackTraceStats();
    TP_MUTEX_LOCKER(s.mutex);
    counts.reserve(s.counts.size());

    for(const auto& p : s.counts)
      counts.emplace_back(formatStackTrace(p.first.frames()), p.second);
  }

  std::sort(counts.begin(), counts.end(), [](const auto& a, const auto& b)
  {
    return a.second < b.second;
  });

  for(const auto& count : counts)
  {
    tpWarning() << "------------------------------------------------------------------------------";
    tpWarning() << "Count: " << count.second << '\n' << count.first;
    tpWarning() << "------------------------------------------------------------------------------";
  }
}
#endif

}
