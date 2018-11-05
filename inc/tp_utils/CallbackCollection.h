#ifndef tp_utils_CallbackCollection_h
#define tp_utils_CallbackCollection_h

#include "tp_utils/Globals.h"

namespace tp_utils
{

//##################################################################################################
template <typename T>
class CallbackCollection;

//##################################################################################################
template<typename T>
class Callback;

//##################################################################################################
template<typename R, typename... Args>
class CallbackCollection<R(Args...)>
{
  template<typename> friend class Callback;
  public:
  using T = R(Args...);

  //################################################################################################
  ~CallbackCollection()
  {
    for(auto unrefCallback : m_unrefCallbacks)
      (*unrefCallback)(this);
  }

  //################################################################################################
  void addCallback(std::function<T>* callback)
  {
    m_callbacks.push_back(callback);
  }

  //################################################################################################
  void removeCallback(std::function<T>* callback)
  {
    tpRemoveOne(m_callbacks, callback);
  }

  //################################################################################################
  R operator()(Args... args) const
  {
    for(auto callback : m_callbacks)
      (*callback)(args...);
    return; //Force void
  }

  private:
  std::vector<std::function<T>*> m_callbacks;
  std::vector<std::function<void(CallbackCollection<T>*)>*> m_unrefCallbacks;
};

//##################################################################################################
template<typename R, typename... Args>
class Callback<R(Args...)>
{
  using C = CallbackCollection<R(Args...)>*;
  public:
  using T = R(Args...);

  //################################################################################################
  Callback(const std::function<T>& callback):
    m_callback(callback),
    m_unrefCallback([&](C c){tpRemoveOne(m_collections, c);})
  {

  }

  //################################################################################################
  ~Callback()
  {
    for(auto c : m_collections)
    {
      tpRemoveOne(c->m_unrefCallbacks, &m_unrefCallback);
      c->removeCallback(&m_callback);
    }
  }

  //################################################################################################
  void connect(CallbackCollection<T>& collection)
  {
    m_collections.push_back(&collection);
    collection.m_unrefCallbacks.push_back(&m_unrefCallback);
    collection.addCallback(&m_callback);
  }

  private:
  std::function<T> m_callback;
  std::vector<C> m_collections;
  std::function<void(C)> m_unrefCallback;
};

}

#endif
