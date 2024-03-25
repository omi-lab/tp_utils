#ifndef tp_utils_TPProperty_h
#define tp_utils_TPProperty_h

#include "tp_utils/CallbackCollection.h"

//##################################################################################################
template<typename T>
struct TPProperty
{
  //################################################################################################
  const T& operator()() const
  {
    return m_data;
  }

  //################################################################################################
  template<typename Closure>
  void update(Closure closure)
  {
    closure(m_data);
    changed();
  }

  //################################################################################################
  void set(const T& data)
  {
    m_data = data;
    changed();
  }

  //################################################################################################
  tp_utils::CallbackCollection<void()> changed;

private:
  T m_data;
};

#endif
