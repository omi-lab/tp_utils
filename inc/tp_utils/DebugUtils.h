#ifndef tp_utils_DebugUtils_h
#define tp_utils_DebugUtils_h

#include "tp_utils/Globals.h"

#include <iostream>
#include <sstream>
#include <functional>
#include <atomic>
#include <vector>
#include <unordered_set>

#define tpWarning tp_utils::DebugHelper(tp_utils::DBGManager::instance().produceWarning())
#define tpDebug tp_utils::DebugHelper(tp_utils::DBGManager::instance().produceDebug())

//##################################################################################################
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
  os << "( ";
  for(const auto& i : v)
    os << i << ' ';
  os << ')';
  return os;
}

//##################################################################################################
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& v)
{
  os << "( ";
  for(const auto& i : v)
    os << i << ' ';
  os << ')';
  return os;
}

namespace tp_utils
{

//##################################################################################################
enum class MessageType
{
  Warning,
  Debug
};

//##################################################################################################
enum class DebugType
{
  Console,
  Table
};

//##################################################################################################
void installMessageHandler(const std::function<void(MessageType, const std::string&)>& callback);

//##################################################################################################
class DebugMode
{
  const std::string& m_classPath;
  DebugType m_debugType;
  std::atomic_bool m_enabled{false};
public:

  //################################################################################################
  DebugMode(const std::string& classPath, DebugType debugType=DebugType::Console);

  //################################################################################################
  ~DebugMode();

  //################################################################################################
  bool operator()();

  //################################################################################################
  //! Sets a large blob to debug that may be served to the user in a single chunk
  void setTable(const std::string& table);

  //################################################################################################
  static void installTableCallback(std::function<void(const std::string&, DebugType, const std::string&)> callback);

  //################################################################################################
  static void enable(const std::string& classPath, DebugType debugType, bool enabled);

  //################################################################################################
  static std::vector<std::string> classPaths(DebugType debugType);
};

//##################################################################################################
class DebugBuffer : public std::stringbuf
{
public:
  //################################################################################################
    int sync() override;
};

//##################################################################################################
struct DBGBase
{
  TP_NONCOPYABLE(DBGBase);

  DBGBase()=default;
  virtual ~DBGBase()=default;
  virtual std::ostream& operator()()=0;
};

//##################################################################################################
struct DBGFactoryBase
{
  TP_NONCOPYABLE(DBGFactoryBase);

  DBGFactoryBase()=default;
  virtual ~DBGFactoryBase()=default;
  virtual DBGBase* produce()=0;
};

//##################################################################################################
template<typename T>
struct DBGFactoryTemplate : public DBGFactoryBase
{
  TP_NONCOPYABLE(DBGFactoryTemplate);

  DBGFactoryTemplate()=default;
  ~DBGFactoryTemplate() override = default;
  DBGBase* produce() override
  {
    return new T();
  }
};

//##################################################################################################
struct DefaultDBG : public DBGBase
{
  TP_NONCOPYABLE(DefaultDBG);

  DefaultDBG();
  ~DefaultDBG()override;
  std::ostream& operator()()override;

  DebugBuffer m_buffer;
  std::ostream m_stream;
};

using DefaultDBGFactory = DBGFactoryTemplate<DefaultDBG>;

//##################################################################################################
struct DebugHelper
{
  TP_NONCOPYABLE(DebugHelper);

  DebugHelper(DBGBase* dbg);
  ~DebugHelper();
  std::ostream& operator()();

  DBGBase* m_dbg;
};

//##################################################################################################
struct DBGManager
{
  DBGManager();
  ~DBGManager();

  void setWarning(DBGFactoryBase* warningFactory);
  DBGBase* produceWarning();

  void setDebug(DBGFactoryBase* debugFactory);
  DBGBase* produceDebug();

  static DBGManager& instance();

  struct Private;
  Private* d;
};

//##################################################################################################
void installDefaultMessageHandler();

}

#endif
