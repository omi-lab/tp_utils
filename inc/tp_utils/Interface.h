#ifndef tp_utils_Interface_h
#define tp_utils_Interface_h

#include "tp_utils/StringID.h"

#include <unordered_map>

#if defined(tp_qt_WIN32)
#undef interface
#endif

namespace tp_utils
{

//##################################################################################################
//! This provides a generic way to pass interfaces to different classes.
class TP_UTILS_SHARED_EXPORT Interface final
{
public:

  //################################################################################################
  //! Find the interface related to the given stringID.
  inline void* findVoid(const tp_utils::StringID& stringID)const
  {
    return tpGetMapValue(m_interfaces, stringID, nullptr);
  }

  //################################################################################################
  template<class T>
  void find(const tp_utils::StringID& stringID, T*& interface)const
  {
    interface = static_cast<T*>(findVoid(stringID));
  }

  //################################################################################################
  inline void set(const tp_utils::StringID& stringID, void* interface)
  {
    m_interfaces[stringID] = interface;
  }

private:
  std::unordered_map<tp_utils::StringID, void*> m_interfaces;
};

}

#endif
