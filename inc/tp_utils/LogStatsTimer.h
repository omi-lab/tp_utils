#ifndef tp_utils_LogStatsTimer_h
#define tp_utils_LogStatsTimer_h

#include "tp_utils/TimeUtils.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/FileUtils.h"
#include "tp_utils/RefCount.h"

#ifdef TP_LINUX
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#endif

#include <functional>
#include <thread>

namespace tp_utils
{

//##################################################################################################
template<class T = void>
struct TP_UTILS_SHARED_EXPORT _LogStatsTimer
{
  //################################################################################################
  _LogStatsTimer(const std::string& path, int64_t intervalMS, const std::function<std::string()>& take, bool append=false)
  {
    m_thread = std::thread([=]
    {
      m_mutex.lock(TPM);
      while(!m_finish)
      {
        m_waitCondition.wait(TPMc m_mutex, intervalMS);
        if(!m_finish)
        {
          m_mutex.unlock(TPM);
          writeTextFile(path, take(), append);
          m_mutex.lock(TPM);
        }
      }
      m_mutex.unlock(TPM);
    });
  }

  //################################################################################################
  ~_LogStatsTimer()
  {
    {
      TP_MUTEX_LOCKER(m_mutex);
      m_finish = true;
      m_waitCondition.wakeAll();
    }

    m_thread.join();
  }

private:
  TPMutex m_mutex{TPM};
  TPWaitCondition m_waitCondition;
  bool m_finish{false};
  std::thread m_thread;
};
using LogStatsTimer = _LogStatsTimer<void>;


#ifdef TP_ENABLE_FUNCTION_TIME
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
#endif

#ifdef TP_ENABLE_MUTEX_TIME
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
#endif

#ifdef TP_REF_COUNT
//##################################################################################################
template<class T = void>
struct TP_UTILS_SHARED_EXPORT _SaveRefCountTimer : public LogStatsTimer
{
  _SaveRefCountTimer(const std::string& path, int64_t intervalMS):
    LogStatsTimer(path, intervalMS, RefCount::takeResults)
  {

  }
};
using SaveRefCountTimer = _SaveRefCountTimer<void>;
#endif

//##################################################################################################
template<class T = void>
class _KeyValueLogStatsTimer
{
public:

  //################################################################################################
  _KeyValueLogStatsTimer(const std::string& path, int64_t intervalMS):
    m_logStatsTimer(path, intervalMS, [&]{return take();}, true)
  {

  }

  //################################################################################################
  void addProducer(const std::string& prefix, const std::function<std::map<std::string, size_t>()>& producer)
  {
    TP_MUTEX_LOCKER(m_mutex);
    m_producers.push_back({prefix, producer});
  }

  //################################################################################################
  std::string take()
  {
    TP_MUTEX_LOCKER(m_mutex);
    std::string results = "==================\n";
    for(const auto& producer : m_producers)
      for(const auto& result : producer.second())
        results += producer.first + result.first + " ---> " + std::to_string(result.second) + '\n';
    return results;
  }

private:
  TPMutex m_mutex{TPM};
  std::vector<std::pair<std::string, std::function<std::map<std::string, size_t>()>>> m_producers;
  tp_utils::LogStatsTimer m_logStatsTimer;
};
using KeyValueLogStatsTimer = _KeyValueLogStatsTimer<void>;


//##################################################################################################
struct VirtualMemory
{
  size_t VmSize{0};
  size_t VmHWM{0};
  size_t VmRSS{0};

  //################################################################################################
  VirtualMemory()
  {
#ifdef TP_LINUX
  //https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
  auto parseLine = [](char* line)
  {
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
  };

  //Note: this value is in KB!
  auto getValue = [=](const std::string& key)
  {
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
      if (strncmp(line, key.c_str(), key.size()) == 0){
        result = parseLine(line);
        break;
      }
    }
    fclose(file);
    return result;
  };

  VmSize = size_t(getValue("VmSize:"));
  VmHWM = size_t(getValue("VmHWM:"));
  VmRSS = size_t(getValue("VmRSS:"));
#endif
  }
};

//##################################################################################################
inline void addMemoryUsageProducer(KeyValueLogStatsTimer& keyValueStatsTimer)
{
#ifdef TP_LINUX
  keyValueStatsTimer.addProducer("Memory ", [=]
  {
    VirtualMemory virtualMemory;

    std::map<std::string, size_t> result;
    result["VmSize"] = virtualMemory.VmSize;
    result["VmHWM"]  = virtualMemory.VmHWM;
    result["VmRSS"]  = virtualMemory.VmRSS;
    return result;
  });
#endif
}

}

#endif

