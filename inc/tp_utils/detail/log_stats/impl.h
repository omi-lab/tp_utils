#ifndef tp_utils_LogStatsTimer_impl_h
#define tp_utils_LogStatsTimer_impl_h

#include "tp_utils/MutexUtils.h"
#include "tp_utils/FileUtils.h"

#include <thread>

namespace tp_utils
{

namespace detail
{
//##################################################################################################
template<class T = void>
struct TP_UTILS_EXPORT _LogStatsTimer
{
  //################################################################################################
  _LogStatsTimer(const std::string& path, int64_t intervalMS, const std::function<std::string()>& take, TPAppend append=TPAppend::No)
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
  _LogStatsTimer(int64_t intervalMS, const std::function<std::string()>& take)
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
          std::cout <<  take();
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
  TP_NONCOPYABLE(_LogStatsTimer);
  TPMutex m_mutex{TPM};
  TPWaitCondition m_waitCondition;
  bool m_finish{false};
  std::thread m_thread;
};


//##################################################################################################
using LogStatsTimer = _LogStatsTimer<void>;

//##################################################################################################
template<class T = void>
class _KeyValueLogStatsTimer
{
public:

  //################################################################################################
  _KeyValueLogStatsTimer(const std::string& path, int64_t intervalMS):
    m_logStatsTimer(path, intervalMS, [&]{return take();}, TPAppend::Yes)
  {

  }

  //################################################################################################
  _KeyValueLogStatsTimer(int64_t intervalMS):
    m_logStatsTimer(intervalMS, [&]{return take();})
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
    std::string lineStart="@LST@";
    std::string lineEnd="#LST#\n";

    TP_MUTEX_LOCKER(m_mutex);
    std::string results = lineStart + "==================" + lineEnd;
    for(const auto& producer : m_producers)
      for(const auto& result : producer.second())
        results += lineStart + producer.first + result.first + " ---> " + std::to_string(result.second) + lineEnd;
    return results;
  }

private:
  TPMutex m_mutex{TPM};
  std::vector<std::pair<std::string, std::function<std::map<std::string, size_t>()>>> m_producers;
  LogStatsTimer m_logStatsTimer;
};

//##################################################################################################
using KeyValueLogStatsTimer = _KeyValueLogStatsTimer<void>;

}
}

#endif
