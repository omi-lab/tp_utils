#include "tp_utils/MutexUtils.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/TimeUtils.h"
#include "tp_utils/FileUtils.h"

#include "lib_platform/Polyfill.h"

#include <condition_variable>
#include <functional>
#include <vector>
#include <unordered_map>
#include <thread>

#define MUTEX_NAME_LEN 52
#define SITE_NAME_LEN  43

namespace std
{

template <>
struct hash<std::pair<const char*, int>>
{
  std::size_t operator()(const std::pair<const char*, int>& k) const
  {
    return hash<const void*>()(static_cast<const void*>(k.first)) ^ hash<int>()(k.second);
  }
};

}

//##################################################################################################
struct TPWaitCondition::Private
{
  std::condition_variable_any cv;

};

//##################################################################################################
TPWaitCondition::TPWaitCondition():
  d(new Private())
{

}

//##################################################################################################
TPWaitCondition::~TPWaitCondition()
{
  delete d;
}

//##################################################################################################
#ifdef TP_ENABLE_MUTEX_TIME
bool TPWaitCondition::wait(TPM_Ac TPMutex& lockedMutex, int64_t ms)
{
  struct MutexW
  {
    const char* m_file_tpm;
    int m_line_tpm;
    TPMutex& m_lockedMutex;

    MutexW(const char* file_tpm, int line_tpm, TPMutex& lockedMutex):
      m_file_tpm(file_tpm),
      m_line_tpm(line_tpm),
      m_lockedMutex(lockedMutex)
    {

    }

    void lock()
    {
      m_lockedMutex.lock(m_file_tpm, m_line_tpm);
    }

    void unlock()
    {
      m_lockedMutex.unlock(m_file_tpm, m_line_tpm);
    }

  };

  MutexW mutexW(TPM_Bc lockedMutex);

  if(ms<INT64_MAX)
    return d->cv.wait_for(mutexW, std::chrono::milliseconds(ms)) == std::cv_status::no_timeout;
  else
    d->cv.wait(mutexW);

  return true;
}
#else
bool TPWaitCondition::wait(TPMutex& lockedMutex, int64_t ms)
{
  if(ms<INT64_MAX)
    return d->cv.wait_for(lockedMutex, std::chrono::milliseconds(ms)) == std::cv_status::no_timeout;

  d->cv.wait(lockedMutex);
  return true;
}
#endif

//##################################################################################################
void TPWaitCondition::wakeOne()
{
  d->cv.notify_one();
}

//##################################################################################################
void TPWaitCondition::wakeAll()
{
  d->cv.notify_all();
}

namespace tp_utils
{

namespace
{

//##################################################################################################
std::string fixedWidthKeepRight(std::string data, size_t len, char pad)
{
  if(data.size()>=len)
    return data.substr(data.size()-len);

  size_t missing = size_t(len) - size_t(data.size());
  if(missing>0)
    data = std::string(missing, pad) + data;

  return data;
}

//##################################################################################################
std::string fixedWidthKeepLeft(std::string data, size_t len, char pad)
{
  if(data.size()>=len)
    return data.substr(0, len);

  size_t missing = size_t(len) - size_t(data.size());
  if(missing>0)
    data += std::string(missing, pad);


  return data;
}

//##################################################################################################
//
struct LockSiteDetails_lt
{
  //locationID -> total elapsed time
  std::unordered_map<int, int> blockedBy;

  std::string name;
  int id{0};
  int lockCount{0};
  int wait{0};
  int failCount{0};
  int held{0};
};

//##################################################################################################
//
struct UnlockSiteDetails_lt
{
  std::string name;
  int id{0};
  int unlockCount{0};
  int held{0};
  int heldMax{0};

  //Reset each time takeResults is called
  int heldRecent{0};
  int unlockCountRecent{0};
};

//##################################################################################################
//There is one of these for each instance of a mutex
struct MutexInstanceDetails_ls
{
  size_t mutexDefinition{0};

  size_t holder{0};
  std::thread::id holderThread;

  std::vector<int> waiting;

  //This uses a list to cope with recursive mutexes
  //The locationID is where the mutex was locked
  //thread -> list of (locationID, timer)
  std::unordered_map<std::thread::id, std::vector<std::pair<int, ElapsedTimer*> > > lockTimers;
};

//##################################################################################################
//There is one of these for each location where the mutex is constructed
struct MutexDefinitionDetails_lt
{
  //locationID -> details
  std::unordered_map<int, LockSiteDetails_lt> lockSiteDetails;
  std::unordered_map<int, UnlockSiteDetails_lt> unlockSiteDetails;

  std::string name;
  const char* type{nullptr};
  const char* file{nullptr};
  int line{0};

  int currentInstances{0};
  int instances{0}; //inst (Total not current)
  int lockCount{0}; //lck
  int totalWait{0}; //wt   in ms
  int totalHold{0}; //hld  in ms
};

}

//##################################################################################################
struct LockStats::Private
{
  std::mutex mutex;

  int mutexInstanceCount{0};
  std::unordered_map<int, MutexInstanceDetails_ls> mutexInstances;

  std::vector<MutexDefinitionDetails_lt> mutexDefinitions;
  std::unordered_map<std::pair<const char*, int>, size_t> mutexDefinitionMap;
  std::unordered_map<std::pair<const char*, int>, size_t> locationIDs;
  size_t locationIDCount{0};

  //##################################################################################################
  size_t locationID(const char* file, int line)
  {
    std::pair<const char*, int> pair(file, line);
    size_t locationID = tpGetMapValue(locationIDs, pair, 0);

    if(locationID==0)
    {
      locationIDCount++;
      locationID = locationIDCount;
      locationIDs[pair] = locationID;
    }

    return locationID;
  }
};

//##################################################################################################
int LockStats::init(const char* type, const char* file, int line)
{
  Private* d = instance();
  std::lock_guard<std::mutex> lk(d->mutex);
  TP_UNUSED(lk);

  //Create a new instance
  d->mutexInstanceCount++;
  MutexInstanceDetails_ls& mutexInstanceDetails = d->mutexInstances[d->mutexInstanceCount];

  //Find or create the definition for this mutex
  std::pair<const char*, int> pair(file, line);
  mutexInstanceDetails.mutexDefinition = tpGetMapValue(d->mutexDefinitionMap, pair, SIZE_MAX);
  if(mutexInstanceDetails.mutexDefinition==SIZE_MAX)
  {
    mutexInstanceDetails.mutexDefinition = d->mutexDefinitionMap.size();
    MutexDefinitionDetails_lt mutexDefinition;
    mutexDefinition.type = type;
    mutexDefinition.file = file;
    mutexDefinition.line = line;

    mutexDefinition.name =
        std::string(type) + ":" +
        fixedWidthKeepRight(file + std::to_string(line),
                            MUTEX_NAME_LEN-(std::string(type).size()+1),
                            ' ');

    d->mutexDefinitions.push_back(mutexDefinition);
    d->mutexDefinitionMap[pair] = mutexInstanceDetails.mutexDefinition;
  }

  {
    MutexDefinitionDetails_lt& mutexDefinition = d->mutexDefinitions[mutexInstanceDetails.mutexDefinition];
    mutexDefinition.instances++;
    mutexDefinition.currentInstances++;
  }

  return d->mutexInstanceCount;
}

//##################################################################################################
void LockStats::destroy(int id)
{
  Private* d = instance();
  std::lock_guard<std::mutex> lk(d->mutex);
  TP_UNUSED(lk);

  {
    MutexInstanceDetails_ls& mutexInstanceDetails = d->mutexInstances[id];

    {
      MutexDefinitionDetails_lt& mutexDefinition = d->mutexDefinitions[mutexInstanceDetails.mutexDefinition];
      mutexDefinition.currentInstances--;
    }
  }

  d->mutexInstances.erase(id);
}

//##################################################################################################
int LockStats::waiting(int id, const char* file, int line)
{
  Private* d = instance();
  std::lock_guard<std::mutex> lk(d->mutex);
  TP_UNUSED(lk);

  MutexInstanceDetails_ls& mutexInstanceDetails = d->mutexInstances[id];
  mutexInstanceDetails.waiting.push_back(d->locationID(file, line));
  return mutexInstanceDetails.holder;
}

//##################################################################################################
void LockStats::locked(int id, const char* file, int line, int elapsedWaiting, int blockingID)
{
  Private* d = instance();
  std::lock_guard<std::mutex> lk(d->mutex);
  TP_UNUSED(lk);

  int locationID = d->locationID(file, line);

  std::thread::id threadID = std::this_thread::get_id();

  MutexInstanceDetails_ls& mutexInstanceDetails = d->mutexInstances[id];
  mutexInstanceDetails.holder = locationID;
  mutexInstanceDetails.holderThread = threadID;

  //We have a list of timers to cope with recursive mutexes
  auto timer = new ElapsedTimer();
  timer->start();

  mutexInstanceDetails.lockTimers[threadID]
      .push_back(std::pair<int, ElapsedTimer*>(locationID, timer));

  tpRemoveOne(mutexInstanceDetails.waiting, locationID);

  {
    MutexDefinitionDetails_lt& mutexDefinition = d->mutexDefinitions[mutexInstanceDetails.mutexDefinition];
    mutexDefinition.lockCount++;
    mutexDefinition.totalWait+=elapsedWaiting;

    {
      LockSiteDetails_lt& lockSiteDetails = mutexDefinition.lockSiteDetails[locationID];
      lockSiteDetails.id = locationID;

      lockSiteDetails.name = fixedWidthKeepRight(std::string(file) + std::to_string(line),
                                                 SITE_NAME_LEN,
                                                 ' ');
      lockSiteDetails.lockCount++;
      lockSiteDetails.wait+=elapsedWaiting;
      if(blockingID>0)
        lockSiteDetails.blockedBy[blockingID]+=elapsedWaiting;
    }
  }
}

//##################################################################################################
void LockStats::tryLock(int id, const char* file, int line, int elapsedWaiting, int blockingID, bool got)
{
  Private* d = instance();
  std::lock_guard<std::mutex> lk(d->mutex);
  TP_UNUSED(lk);

  int locationID = d->locationID(file, line);

  MutexInstanceDetails_ls& mutexInstanceDetails = d->mutexInstances[id];
  if(got)
  {
    std::thread::id threadID = std::this_thread::get_id();

    mutexInstanceDetails.holder = locationID;
    mutexInstanceDetails.holderThread = threadID;

    //We have a list of timers to cope with recursive mutexes
    auto timer = new ElapsedTimer();
    timer->start();
    mutexInstanceDetails.lockTimers[threadID]
        .push_back(std::pair<int, ElapsedTimer*>(locationID, timer));
  }
  tpRemoveOne(mutexInstanceDetails.waiting, locationID);

  {
    MutexDefinitionDetails_lt& mutexDefinition = d->mutexDefinitions[mutexInstanceDetails.mutexDefinition];
    if(got)
      mutexDefinition.lockCount++;
    mutexDefinition.totalWait+=elapsedWaiting;

    {
      LockSiteDetails_lt& lockSiteDetails = mutexDefinition.lockSiteDetails[locationID];
      lockSiteDetails.id = locationID;
      lockSiteDetails.name = fixedWidthKeepRight(std::string(file) + std::to_string(line),
                                                 SITE_NAME_LEN,
                                                 ' ');
      if(got)
        lockSiteDetails.lockCount++;
      else
        lockSiteDetails.failCount++;
      lockSiteDetails.wait+=elapsedWaiting;
      if(blockingID>0)
        lockSiteDetails.blockedBy[blockingID]+=elapsedWaiting;
    }
  }
}

//##################################################################################################
void LockStats::unlock(int id, const char* file, int line)
{
  Private* d = instance();
  std::lock_guard<std::mutex> lk(d->mutex);
  TP_UNUSED(lk);

  int locationID = d->locationID(file, line);

  MutexInstanceDetails_ls& mutexInstanceDetails = d->mutexInstances[id];
  std::thread::id threadID = std::this_thread::get_id();

  if(mutexInstanceDetails.holderThread == threadID)
  {
    mutexInstanceDetails.holder = 0;
    mutexInstanceDetails.holderThread = std::thread::id();
  }

  {
    MutexDefinitionDetails_lt& mutexDefinition = d->mutexDefinitions[mutexInstanceDetails.mutexDefinition];
    int64_t elapsed=0;
    bool empty=false;
    int lockLocationID=0;
    {
      std::vector<std::pair<int, ElapsedTimer*>>& timerList = mutexInstanceDetails.lockTimers[threadID];
      if(!timerList.empty())
      {
        const auto& timer = tpTakeLast(timerList);
        lockLocationID=timer.first;
        elapsed=timer.second->elapsed();
        delete timer.second;
      }
      else
        tpWarning() << "Failed to find timer for locked mutex: " << file << line;

      if(timerList.empty())
      {
        empty=true;
        mutexInstanceDetails.lockTimers.erase(threadID);
      }
    }

    //Only alter the total if this is not recursive or the outer most lock/unlock, eg.
    //mutex(recursive)
    //mutex.lock()
    //mutex.lock()
    //mutex.unlock()
    //mutex.unlock() <-- Only update the total here
    if(empty)
      mutexDefinition.totalHold+=elapsed;

    if(lockLocationID>0)
    {
      LockSiteDetails_lt& lockSiteDetails = mutexDefinition.lockSiteDetails[lockLocationID];
      lockSiteDetails.id = lockLocationID;
      lockSiteDetails.held+=elapsed;
    }

    {
      UnlockSiteDetails_lt& unlockSiteDetails = mutexDefinition.unlockSiteDetails[locationID];
      unlockSiteDetails.id = locationID;
      unlockSiteDetails.name = fixedWidthKeepRight(std::string(file) + std::to_string(line),
                                                   SITE_NAME_LEN,
                                                   ' ');
      unlockSiteDetails.held+=elapsed;

      if(elapsed>unlockSiteDetails.heldMax)
        unlockSiteDetails.heldMax=int(elapsed);

      unlockSiteDetails.unlockCount++;
      unlockSiteDetails.heldRecent+=elapsed;
      unlockSiteDetails.unlockCountRecent++;
    }
  }
}

//##################################################################################################
std::string LockStats::takeResults()
{
  Private* d = instance();
  std::lock_guard<std::mutex> lk(d->mutex);
  TP_UNUSED(lk);

  std::string result;

  //-- Sort the mutexes by wt ----------------------------------------------------------------------
  std::vector<MutexDefinitionDetails_lt*> sortedMutexDefinitions;
  for(MutexDefinitionDetails_lt& mutexDefinition : d->mutexDefinitions)
  {
    int c=0;
    while(c<int(sortedMutexDefinitions.size()) && sortedMutexDefinitions.at(c)->totalWait>=mutexDefinition.totalWait)
      c++;

    sortedMutexDefinitions.insert(sortedMutexDefinitions.begin()+c, &mutexDefinition);
  }

  std::string titleLineLock = "+---+";
  std::string titleLock     = "|ID |";
  titleLineLock += fixedWidthKeepLeft("", SITE_NAME_LEN, '-');
  titleLock     += fixedWidthKeepLeft("Lock name", SITE_NAME_LEN, ' ');
  titleLineLock += std::string("+----------+----------+----------+----------+----------------------------+\n");
  titleLock     += std::string("|Lock count|Wait (ms) |Held (ms) |Held avg  |Blocked by (f: tryLock fail)|\n");

  std::string titleLineUnlock = "+---+";
  std::string titleUnlock     = "|ID |";
  titleLineUnlock += fixedWidthKeepLeft("", SITE_NAME_LEN, '-');
  titleUnlock     += fixedWidthKeepLeft("Unlock name", SITE_NAME_LEN, ' ');
  titleLineUnlock += "+----------+----------+----------+----------+----------+--------+--------+\n";
  titleUnlock     += "|Unlock cnt|Waitin cnt|Held (ms) |Held avg  |Held max  |Held rc |Count rc|\n";

  //-- Print out the details for each mutex --------------------------------------------------------
  for(MutexDefinitionDetails_lt* mutexDefinition : tpConst(sortedMutexDefinitions))
  {
    //Totals
    {
      std::string name = mutexDefinition->name;
      std::string inst = fixedWidthKeepRight(std::to_string(mutexDefinition->instances), 10, '0');
      std::string lck  = fixedWidthKeepRight(std::to_string(mutexDefinition->lockCount), 10, '0');
      std::string wt   = fixedWidthKeepRight(std::to_string(mutexDefinition->totalWait), 10, '0');
      std::string hld  = fixedWidthKeepRight(std::to_string(mutexDefinition->totalHold), 10, '0');
      result+="\n"+name+" Totals(inst:"+inst+",lck:"+lck+",wt:"+wt+",hld:"+hld+")\n";
    }

    //.. Lock sites ................................................................................
    if(!mutexDefinition->lockSiteDetails.empty())
    {
      result.append(titleLineLock);
      result.append(titleLock);
      result.append(titleLineLock);

      std::vector<int> keys;
      for(const auto& i : tpConst(mutexDefinition->lockSiteDetails))
        keys.push_back(i.first);

      std::sort(keys.begin(), keys.end());

      for(int key : tpConst(keys))
      {
        LockSiteDetails_lt& lockSite = mutexDefinition->lockSiteDetails[key];

        int heldAverage = lockSite.held;
        if(lockSite.lockCount>0)
          heldAverage = heldAverage / lockSite.lockCount;

        std::string blockedByString;

        std::string id        = fixedWidthKeepRight(std::to_string(lockSite.id),         3, '0');
        std::string name      =                                    lockSite.name                ;
        std::string lockCount = fixedWidthKeepRight(std::to_string(lockSite.lockCount), 10, '0');
        std::string wait      = fixedWidthKeepRight(std::to_string(lockSite.wait),      10, '0');
        std::string held      = fixedWidthKeepRight(std::to_string(lockSite.held),      10, '0');
        std::string heldAvg   = fixedWidthKeepRight(std::to_string(heldAverage),        10, '0');
        std::string blockedBy = fixedWidthKeepLeft(blockedByString,                      28, ' ');
        result+="|"+id+"|"+name+"|"+lockCount+"|"+wait+"|"+held+"|"+heldAvg+"|"+blockedBy+"|\n";
      }
      result.append(titleLineLock);
    }

    //.. Unlock sites ..............................................................................
    if(!mutexDefinition->unlockSiteDetails.empty())
    {
      result.append(titleLineUnlock);
      result.append(titleUnlock);
      result.append(titleLineUnlock);

      std::vector<int> keys;
      for(const auto& i : tpConst(mutexDefinition->unlockSiteDetails))
        keys.push_back(i.first);

      std::sort(keys.begin(), keys.end());

      for(int key : tpConst(keys))
      {
        UnlockSiteDetails_lt& unlockSite = mutexDefinition->unlockSiteDetails[key];

        int heldAverage = unlockSite.held;
        if(unlockSite.unlockCount>0)
          heldAverage = heldAverage / unlockSite.unlockCount;

        std::string id        = fixedWidthKeepRight(std::to_string(unlockSite.id),                 3, '0');
        std::string name      =                                    unlockSite.name                        ;
        std::string lockCount = fixedWidthKeepRight(std::to_string(unlockSite.unlockCount),       10, '0');
        std::string waitinCnt = fixedWidthKeepRight(std::to_string(0),                            10, '0');
        std::string held      = fixedWidthKeepRight(std::to_string(unlockSite.held),              10, '0');
        std::string heldAvg   = fixedWidthKeepRight(std::to_string(heldAverage),                  10, '0');
        std::string heldMax   = fixedWidthKeepRight(std::to_string(unlockSite.heldMax),           10, '0');
        std::string heldRc    = fixedWidthKeepRight(std::to_string(unlockSite.heldRecent),         8, '0');
        std::string countRc   = fixedWidthKeepRight(std::to_string(unlockSite.unlockCountRecent),  8, '0');

        unlockSite.heldRecent = 0;
        unlockSite.unlockCountRecent = 0;
        result+="|"+id+"|"+name+"|"+lockCount+"|"+waitinCnt+"|"+held+"|"+heldAvg+"|"+heldMax+"|"+heldRc+"|"+countRc+"|\n";
      }
      result.append(titleLineUnlock);
    }
  }

  return result;
}

//##################################################################################################
struct SaveLockStatsTimer::Private
{
  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
  bool finish{false};
  std::thread thread;
};

//##################################################################################################
SaveLockStatsTimer::SaveLockStatsTimer(const std::string& path, int64_t intervalMS):
  d(new  Private)
{
  d->thread = std::thread([=]
  {
    d->mutex.lock(TPM);
    while(!d->finish)
    {
      d->waitCondition.wait(TPMc d->mutex, intervalMS);
      if(!d->finish)
      {
        d->mutex.unlock(TPM);
        writeTextFile(path, LockStats::takeResults());
        d->mutex.lock(TPM);
      }
    }
    d->mutex.unlock(TPM);
  });
}

//##################################################################################################
SaveLockStatsTimer::~SaveLockStatsTimer()
{
  {
    TP_MUTEX_LOCKER(d->mutex);
    d->finish = true;
    d->waitCondition.wakeAll();
  }

  d->thread.join();

  delete d;
}

//##################################################################################################
LockStats::Private* LockStats::instance()
{
  static LockStats::Private instance;
  return &instance;
}

}
