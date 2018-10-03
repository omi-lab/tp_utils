#include "tp_utils/DebugUtils.h"
#include "tp_utils/MutexUtils.h"

#ifdef TDP_ANDROID
#include <android/log.h>
#endif

namespace tp_utils
{
namespace
{

TPMutex debugMutex{TPM};
std::function<void(MessageType, const std::string&)> debugCallback;
std::function<void(const std::string&, DebugType, const std::string&)> tableCallback;
std::unordered_map<std::string, std::unordered_map<int, bool>> enabledDebugModeObjects;
std::vector<DebugMode*> debugModeObjects;
}

//##################################################################################################
void installMessageHandler(const std::function<void(MessageType, const std::string&)>& callback)
{
  TP_MUTEX_LOCKER(debugMutex);
  debugCallback = callback;
}

//##################################################################################################
DebugMode::DebugMode(const std::string& classPath, DebugType debugType):
  m_classPath(classPath),
  m_debugType(debugType)
{
  TP_MUTEX_LOCKER(debugMutex);
  debugModeObjects.push_back(this);
  auto i = enabledDebugModeObjects.find(classPath);
  if(i != enabledDebugModeObjects.end())
  {
    auto ii = i->second.find(int(debugType));
    if(ii != i->second.end())
      m_enabled = ii->second;
  }
}

//##################################################################################################
DebugMode::~DebugMode()
{
  TP_MUTEX_LOCKER(debugMutex);
  tpRemoveOne(debugModeObjects, this);
}

//##################################################################################################
bool DebugMode::operator()()
{
  return m_enabled;
}

//##################################################################################################
void DebugMode::setTable(const std::string& table)
{
  if(m_enabled)
  {
    TP_MUTEX_LOCKER(debugMutex);
    if(tableCallback)
      tableCallback(m_classPath, m_debugType, table);
  }
}

//##################################################################################################
void DebugMode::installTableCallback(std::function<void(const std::string&, DebugType, const std::string&)> callback)
{
  TP_MUTEX_LOCKER(debugMutex);
  tableCallback = std::move(callback);
}

//##################################################################################################
void DebugMode::enable(const std::string& classPath, DebugType debugType, bool enabled)
{
  TP_MUTEX_LOCKER(debugMutex);
  enabledDebugModeObjects[classPath][int(debugType)] = enabled;
  for(DebugMode* dm : debugModeObjects)
    if(dm->m_classPath == classPath && dm->m_debugType == debugType)
      dm->m_enabled = enabled;
}

//##################################################################################################
std::vector<std::string> DebugMode::classPaths(DebugType debugType)
{
  TP_MUTEX_LOCKER(debugMutex);
  std::vector<std::string> classPaths;

  for(DebugMode* dm : debugModeObjects)
    if(dm->m_debugType == debugType)
      classPaths.push_back(dm->m_classPath);

  return classPaths;
}

//##################################################################################################
int DebugBuffer::sync()
{
  TP_MUTEX_LOCKER(debugMutex);
  if(debugCallback)
    debugCallback(MessageType::Warning, str());
  else
  {
    std::cout << str();
    std::cout.flush();
  }

  str("");
  return 0;
}

//##################################################################################################
DefaultDBG::DefaultDBG():
  m_stream(&m_buffer)
{

}

//##################################################################################################
DefaultDBG::~DefaultDBG()
{
  m_stream << std::endl;
}

//##################################################################################################
std::ostream& DefaultDBG::operator()()
{
  return m_stream;
}

//##################################################################################################
DebugHelper::DebugHelper(DBGBase* dbg):
  m_dbg(dbg)
{

}

//##################################################################################################
DebugHelper::~DebugHelper()
{
  delete m_dbg;
}

//##################################################################################################
std::ostream& DebugHelper::operator()()
{
  return (*m_dbg)();
}

//##################################################################################################
struct DBGManager::Private
{
  std::mutex mutex;
  DBGFactoryBase* warningFactory{new DefaultDBGFactory()};
  DBGFactoryBase* debugFactory{new DefaultDBGFactory()};
};

//##################################################################################################
DBGManager::DBGManager():
  d(new Private())
{

}

//##################################################################################################
DBGManager::~DBGManager()
{
  delete d;
}

//##################################################################################################
void DBGManager::setWarning(DBGFactoryBase* warningFactory)
{
  std::lock_guard<std::mutex> lg(d->mutex);
  TP_UNUSED(lg);
  delete d->warningFactory;
  d->warningFactory = warningFactory;
}

//##################################################################################################
DBGBase* DBGManager::produceWarning()
{
  std::lock_guard<std::mutex> lg(d->mutex);
  TP_UNUSED(lg);
  return d->warningFactory->produce();
}

//##################################################################################################
void DBGManager::setDebug(DBGFactoryBase* debugFactory)
{
  std::lock_guard<std::mutex> lg(d->mutex);
  TP_UNUSED(lg);
  delete d->debugFactory;
  d->debugFactory = debugFactory;
}

//##################################################################################################
DBGBase* DBGManager::produceDebug()
{
  std::lock_guard<std::mutex> lg(d->mutex);
  TP_UNUSED(lg);
  return d->debugFactory->produce();
}

//##################################################################################################
DBGManager& DBGManager::instance()
{
  static DBGManager instance;
  return instance;
}

//##################################################################################################
//## Platform Abstractions #########################################################################
//##################################################################################################

#ifdef TDP_ANDROID
namespace
{
//##################################################################################################
void messageHandler(tp_utils::MessageType messageType, const std::string& message)
{
  const char* tag="tpDebug";
  switch(messageType)
  {
  case tp_utils::MessageType::Debug:   tag="tpDebug";   break;
  case tp_utils::MessageType::Warning: tag="tpWarning"; break;
  }
  __android_log_print(ANDROID_LOG_DEBUG, tag, "%s", message.c_str());
}
}
#endif

//##################################################################################################
void installDefaultMessageHandler()
{
#ifdef __ANDROID_API__
  tp_utils::installMessageHandler(messageHandler);
#endif
}

}
