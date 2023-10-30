#include "tp_utils/TimeUtils.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/MutexUtils.h"

#ifdef TP_ENABLE_TIME_SCOPE
#include "tp_utils/FileUtils.h"
#include "json.hpp"
#endif

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
  int64_t mainThreadTotal{0};
};

const std::thread::id mainThreadId = std::this_thread::get_id();
}

//##################################################################################################
struct FunctionTimeStats::Private
{
  TPMutex mutex{TPM};
  std::unordered_map<std::string, FunctionTimeStatsDetails_lt> stats;
};

//##################################################################################################
void FunctionTimeStats::add(const std::vector<FunctionTimeReading>& readings)
{
  auto i = instance();
  TP_MUTEX_LOCKER(i->mutex);

  for(const auto& reading : readings)
  {
    auto& s = i->stats[reading.key];
    s.count++;
    s.max = tpMax(s.max, reading.timeTaken);
    s.total += reading.timeTaken;

    if(mainThreadId == std::this_thread::get_id())
      s.mainThreadTotal += reading.timeTaken;
  }
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
  size_t f=20;

  auto addLine = [&]
  {
    result += '+' + std::string(a, '-') + '+' + std::string(b, '-') + '+' + std::string(c, '-') + '+' + std::string(d, '-') + '+' + std::string(e, '-') + '+' + std::string(f, '-') + "+\n";
  };

  addLine();

  result += '|' + fixedWidthKeepLeft("Name", a, ' ') + '|';
  result += fixedWidthKeepLeft("Count", b, ' ') + '|';
  result += fixedWidthKeepLeft("Total ms", c, ' ') + '|';
  result += fixedWidthKeepLeft("Max ms", d, ' ') + '|';
  result += fixedWidthKeepLeft("Main thread ms", e, ' ') + '|';
  result += fixedWidthKeepLeft("Average micro", f, ' ') + "|\n";

  addLine();

  for(const auto& details : detailsList)
  {
    auto average = details.second.total / details.second.count;

    result += '|' + details.first + '|';
    result += fixedWidthKeepRight(std::to_string(details.second.count               ), b, ' ') + '|';
    result += fixedWidthKeepRight(std::to_string(details.second.total          /1000), c, ' ') + '|';
    result += fixedWidthKeepRight(std::to_string(details.second.max            /1000), d, ' ') + '|';
    result += fixedWidthKeepRight(std::to_string(details.second.mainThreadTotal/1000), e, ' ') + '|';
    result += fixedWidthKeepRight(std::to_string(average                            ), f, ' ') + "|\n";
  }

  addLine();

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
bool FunctionTimeStats::isMainThread()
{
  return mainThreadId == std::this_thread::get_id();
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

namespace
{
struct Step_lt
{
  size_t level{0};
  std::string name;
  int64_t finishTime{0};
  int64_t timeTaken{0};
};

thread_local std::vector<FunctionTimeReading> readings;

#ifdef TP_ENABLE_TIME_SCOPE
thread_local std::vector<Step_lt> steps;
thread_local size_t level{0};
thread_local size_t fileIndex{0};
#endif
}


//##################################################################################################
FunctionTimer::FunctionTimer(const char* file, int line, const char* name):
  m_index(readings.size())
{
  auto& reading = readings.emplace_back();
  reading.start = currentTimeMicroseconds();
  reading.key = fixedWidthKeepRight(std::string(file) + ':' + std::to_string(line) + ' ' + name, 50, ' ');

#ifdef TP_ENABLE_TIME_SCOPE

  {
    m_stepIndex = steps.size();
    auto& step = steps.emplace_back();

    step.level = level;
    level++;

    step.name = reading.key;
    step.finishTime = reading.start;
  }
#endif
}

//##################################################################################################
FunctionTimer::~FunctionTimer()
{
  auto time = currentTimeMicroseconds();

#ifdef TP_ENABLE_TIME_SCOPE
  level--;
#endif

  {
    auto& reading = readings.at(m_index);
    reading.timeTaken = time - reading.start;
  }

#ifdef TP_ENABLE_TIME_SCOPE
  {
    auto& step = steps[m_stepIndex];
    step.timeTaken = time - step.finishTime;
    step.finishTime = time;
  }
#endif


  if(m_index == 0)
  {
    TP_CLEANUP([&]{readings.clear();});

#ifdef TP_ENABLE_TIME_SCOPE
    TP_CLEANUP([&]{level=0;});
#endif

    FunctionTimeStats::add(readings);

#ifdef TP_ENABLE_TIME_SCOPE
    TP_CLEANUP([&]{steps.clear();});


    if(FunctionTimeStats::isMainThread())
    {
      if(auto timeTakenMilliseconds = readings.at(0).timeTaken/1000; timeTakenMilliseconds>=20)
      {
        auto printStep = [](const auto& s)
        {
          tpWarning() << fileIndex << " " << std::string(s.level*2, ' ') << s.name << ": " << s.timeTaken;
        };
#if 0
        {
          for(const auto& step : steps)
            printStep(step);
        }
#else
        {
          if(!steps.empty())
            printStep(steps.front());

          nlohmann::json root = nlohmann::json::array();

          std::vector<nlohmann::json*> levels;
          levels.push_back(&root);

          for(const auto& step : steps)
          {
            levels.resize(step.level+1);

            {
              levels.back()->emplace_back();
              nlohmann::json& j = levels.back()->back();
              j["name"] = step.name;
              j["timeTaken"] = step.timeTaken;
              j["children"] = nlohmann::json::array();
              levels.emplace_back(&j["children"]);
            }
          }

          tp_utils::writeJSONFile(TP_ENABLE_TIME_SCOPE + fixedWidthKeepRight(std::to_string(fileIndex), 8, '0') + ".json", root, 2);
          fileIndex++;
        }
#endif
      }
    }
#endif
  }
}

//##################################################################################################
void FunctionTimer::finishStep(const char* name)
{
#ifndef TP_ENABLE_TIME_SCOPE
  TP_UNUSED(name);
#else
  auto& step = steps[m_stepIndex];
  auto finishTime = currentTimeMicroseconds();
  auto startTime = step.finishTime;
  if(steps.size()>0)
  {
    for(size_t i=steps.size()-1; i>=m_stepIndex; i--)
    {
      const auto& previousStep = steps.at(i);
      if(step.level == previousStep.level)
      {
        startTime = previousStep.finishTime;
        break;
      }
    }
  }

  {
    auto& newStep = steps.emplace_back();
    newStep.level = step.level+1;
    newStep.name = name;
    newStep.finishTime = finishTime;
    newStep.timeTaken = finishTime - startTime;
  }
#endif
}

//##################################################################################################
void FunctionTimer::printStack(const char* name)
{
#ifndef TP_ENABLE_TIME_SCOPE
  TP_UNUSED(name);
#else
 size_t l = level;
 tpWarning() << "---- FunctionTimer::printStack ----";
 tpWarning() << name;
 for(size_t i=steps.size()-1; i<steps.size(); i--)
 {
   if(const auto& s = steps.at(i); s.level==l)
   {
     l--;
     tpWarning() << "  - " << s.name << ": " << s.timeTaken;
   }
 }

 tpWarning() << "-----------------------------------";
#endif
}

#endif

}
