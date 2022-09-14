#include "tp_utils/TimeUtils.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/MutexUtils.h"

#include <chrono>
#include <thread>

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
int64_t currentTimeMicroseconds()
{
  return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

//##################################################################################################
struct ElapsedTimer::Private
{
  std::chrono::steady_clock::time_point start;
  int64_t smallTime;

  //################################################################################################
  Private(int64_t smallTime_):
    smallTime(smallTime_)
  {

  }
};

//##################################################################################################
ElapsedTimer::ElapsedTimer(int64_t smallTime):
  d(new Private(smallTime))
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
int64_t ElapsedTimer::elapsed() const
{
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now - d->start).count();
}

//##################################################################################################
void ElapsedTimer::printTime(const char* msg)
{
  auto e = restart();
  if(e>d->smallTime)
    tpWarning() << msg << " (" << e << ")";
}

#ifdef TP_ENABLE_FUNCTION_TIME

namespace
{
struct FunctionTimeStatsDetails_lt
{
  int64_t count{0};
  int64_t max{0};
  int64_t total{0};
};
}

//##################################################################################################
struct FunctionTimeStats::Private
{
  TPMutex mutex{TPM};
  std::unordered_map<std::string, FunctionTimeStatsDetails_lt> stats;
};

//##################################################################################################
void FunctionTimeStats::add(int64_t timeMicroseconds, const char* file, int line, const std::string& name)
{
  auto i = instance();
  TP_MUTEX_LOCKER(i->mutex);
  auto key = fixedWidthKeepRight(std::string(file) + ':' + std::to_string(line) + ' ' + name, 50, ' ');
  auto& s = i->stats[key];
  s.count++;
  s.max = tpMax(s.max, timeMicroseconds);
  s.total += timeMicroseconds;
}

//##################################################################################################
std::string FunctionTimeStats::takeResults()
{
  std::string result;

  auto i = instance();
  TP_MUTEX_LOCKER(i->mutex);

  std::vector<std::pair<std::string, FunctionTimeStatsDetails_lt>> detailsList;
  detailsList.reserve(i->stats.size());
  for(const auto& it : i->stats)
    detailsList.push_back({it.first, it.second});

  std::sort(detailsList.begin(), detailsList.end(), [](const std::pair<std::string, FunctionTimeStatsDetails_lt>& a, const std::pair<std::string, FunctionTimeStatsDetails_lt>& b)
  {
    return a.second.total > b.second.total;
  });

  size_t a=50;
  size_t b=10;
  size_t c=20;
  size_t d=20;
  size_t e=20;

  result += '+' + std::string(a, '-') + '+' + std::string(b, '-') + '+' + std::string(c, '-') + '+' + std::string(d, '-') + '+' + std::string(e, '-') + "+\n";

  result += '|' + fixedWidthKeepLeft("Name", a, ' ') + '|';
  result += fixedWidthKeepLeft("Count", b, ' ') + '|';
  result += fixedWidthKeepLeft("Total ms", c, ' ') + '|';
  result += fixedWidthKeepLeft("Max ms", d, ' ') + '|';
  result += fixedWidthKeepLeft("Average micro", e, ' ') + "|\n";

  result += '+' + std::string(a, '-') + '+' + std::string(b, '-') + '+' + std::string(c, '-') + '+' + std::string(d, '-') + '+' + std::string(e, '-') + "+\n";

  for(const auto& details : detailsList)
  {
    auto average = details.second.total / details.second.count;

    result += '|' + details.first + '|';
    result += fixedWidthKeepRight(std::to_string(details.second.count     ), b, ' ') + '|';
    result += fixedWidthKeepRight(std::to_string(details.second.total/1000), c, ' ') + '|';
    result += fixedWidthKeepRight(std::to_string(details.second.max  /1000), d, ' ') + '|';
    result += fixedWidthKeepRight(std::to_string(average                  ), e, ' ') + "|\n";
  }
  result += '+' + std::string(a, '-') + '+' + std::string(b, '-') + '+' + std::string(c, '-') + '+' + std::string(d, '-') + '+' + std::string(e, '-') + "+\n";

  return result;
}

//##################################################################################################
void FunctionTimeStats::reset()
{
  auto i = instance();
  TP_MUTEX_LOCKER(i->mutex);
  i->stats.clear();
}

//##################################################################################################
std::map<std::string, size_t> FunctionTimeStats::keyValueResults()
{
  std::map<std::string, size_t> result;

  auto i = instance();
  TP_MUTEX_LOCKER(i->mutex);
  for(const auto& it : i->stats)
  {
    result[it.first+"_count"] = it.second.count;
    result[it.first+"_max"  ] = it.second.max;
    result[it.first+"_total"] = it.second.total;
    result[it.first+"_average"] = it.second.total/it.second.count;
  }

  return result;
}

//##################################################################################################
FunctionTimeStats::Private* FunctionTimeStats::instance()
{
  static FunctionTimeStats::Private instance;
  return &instance;
}

//##################################################################################################
FunctionTimer::FunctionTimer(const char* file, int line, const std::string& name):
  m_start(currentTimeMicroseconds()),
  m_file(file),
  m_line(line),
  m_name(name)
{

}

//##################################################################################################
FunctionTimer::~FunctionTimer()
{
  FunctionTimeStats::add(currentTimeMicroseconds() - m_start, m_file, m_line, m_name);
}

#endif

}
