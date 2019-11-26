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
template<typename T>
struct TP_UTILS_SHARED_EXPORT SaveStatsTimer
{
  //################################################################################################
  SaveStatsTimer(const std::string& path, int64_t intervalMS, const T& take)
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
          writeTextFile(path, take());
          m_mutex.lock(TPM);
        }
      }
      m_mutex.unlock(TPM);
    });
  }

  //################################################################################################
  ~SaveStatsTimer()
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
struct TP_UTILS_SHARED_EXPORT SaveFunctionTimeStatsTimer : public SaveStatsTimer<std::function<std::string()>>
{
  SaveFunctionTimeStatsTimer(const std::string& path, int64_t intervalMS):
    SaveStatsTimer(path, intervalMS, FunctionTimeStats::takeResults)
  {

  }
};
#endif

#ifdef TP_ENABLE_MUTEX_TIME
//##################################################################################################
struct TP_UTILS_SHARED_EXPORT SaveLockStatsTimer : public SaveStatsTimer<std::function<std::string()>>
{
  SaveLockStatsTimer(const std::string& path, int64_t intervalMS):
    SaveStatsTimer(path, intervalMS, LockStats::takeResults)
  {

  }
};
#endif

}

#endif

