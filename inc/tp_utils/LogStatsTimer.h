#ifndef tp_utils_LogStatsTimer_h
#define tp_utils_LogStatsTimer_h

#include "tp_utils/TimeUtils.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/FileUtils.h"

#include <functional>
#include <thread>

namespace tp_utils
{

//##################################################################################################
template<class T = void>
struct TP_UTILS_SHARED_EXPORT LogStatsTimer
{
  //################################################################################################
  LogStatsTimer(const std::string& path, int64_t intervalMS, const std::function<std::string()>& take, bool append=false)
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
  ~LogStatsTimer()
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


#ifdef TP_ENABLE_FUNCTION_TIME
//##################################################################################################
template<class T = void>
struct TP_UTILS_SHARED_EXPORT SaveFunctionTimeStatsTimer : public LogStatsTimer<void>
{
  SaveFunctionTimeStatsTimer(const std::string& path, int64_t intervalMS):
    LogStatsTimer(path, intervalMS, FunctionTimeStats::takeResults)
  {

  }
};
#endif

#ifdef TP_ENABLE_MUTEX_TIME
//##################################################################################################
template<class T = void>
struct TP_UTILS_SHARED_EXPORT SaveLockStatsTimer : public LogStatsTimer<void>
{
  SaveLockStatsTimer(const std::string& path, int64_t intervalMS):
    LogStatsTimer(path, intervalMS, LockStats::takeResults)
  {

  }
};
#endif

//##################################################################################################
template<class T = void>
class KeyValueLogStatsTimer
{
public:

  //################################################################################################
  KeyValueLogStatsTimer(const std::string& path, int64_t intervalMS):
    m_path(path),
    m_intervalMS(intervalMS)
  {

  }

  //################################################################################################
  void addProducer(const std::function<std::map<std::string, size_t>()>& producer)
  {
    TP_MUTEX_LOCKER(m_mutex);
    m_producers.push_back(producer);
  }

  //################################################################################################
  void start()
  {
    m_logStatsTimer = std::make_unique<tp_utils::LogStatsTimer>(m_path, m_intervalMS, [&]{return take();}, true);
  }

  //################################################################################################
  void stop()
  {
    m_logStatsTimer.reset();
  }

  //################################################################################################
  std::string take()
  {
    TP_MUTEX_LOCKER(m_mutex);
    std::string results;
    for(const auto& producer : m_producers)
      for(const auto& result : producer())
        results += result.first + " ---> " + std::to_string(result.second) + '\n';
    return results;
  }

private:
  TPMutex m_mutex{TPM};
  std::vector<std::function<std::map<std::string, size_t>()> > m_producers;
  std::string m_path;
  int64_t m_intervalMS;
  std::unique_ptr<tp_utils::LogStatsTimer<>> m_logStatsTimer;
};

}

#endif

