#ifndef tp_utils_DebugUtils_h
#define tp_utils_DebugUtils_h

#include "tp_utils/Globals.h"

#include <iostream>
#include <sstream>
#include <functional>
#include <atomic>
#include <vector>
#include <unordered_set>

#define tpWarning tp_utils::DebugHelper()
#define tpDebug tp_utils::DebugHelper()

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
struct DebugHelper
{
  DebugBuffer m_buffer;
  std::ostream m_stream;

  //################################################################################################
  DebugHelper();

  //################################################################################################
  ~DebugHelper();

  //################################################################################################
  std::ostream& operator()();
};

}

#endif
