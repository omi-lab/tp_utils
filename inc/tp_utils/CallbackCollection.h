#ifndef tp_utils_CallbackCollection_h
#define tp_utils_CallbackCollection_h

#include "tp_utils/Globals.h"

#include <memory>
#include <cassert>

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

    tpDeleteAll(m_nonRemovableCallbacks);
  }

  //################################################################################################
  void addCallback(std::function<T>* callback)
  {
    assert(callback);
    assert(*callback);
    m_callbacks.push_back(callback);
  }

  //################################################################################################
  void addCallback(std::function<T> callback)
  {
    assert(callback);
    m_nonRemovableCallbacks.push_back(new std::function<T>(callback));
    m_callbacks.push_back(m_nonRemovableCallbacks.back());
  }

  //################################################################################################
  void removeCallback(std::function<T>* callback)
  {
    assert(callback);
    assert(*callback);
    tpRemoveOne(m_callbacks, callback);
  }

  //################################################################################################
  R operator()(Args... args) const
  {
    for(size_t i=0; i<m_callbacks.size(); i++)
      (*m_callbacks.at(i))(args...);

    return; //Force void
  }

  private:
  std::vector<std::function<T>*> m_nonRemovableCallbacks;
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
  Callback():
    m_unrefCallback([&](C c){tpRemoveOne(m_collections, c);})
  {

  }

  //################################################################################################
  Callback(const std::function<T>& callback):
    m_callback(callback),
    m_unrefCallback([&](C c){tpRemoveOne(m_collections, c);})
  {

  }

  //################################################################################################
  template<typename F>
  Callback(const F& callback):
    m_callback(callback),
    m_unrefCallback([&](C c){tpRemoveOne(m_collections, c);})
  {

  }

  //################################################################################################
  ~Callback()
  {
    disconnect();
  }

  //################################################################################################
  void disconnect()
  {
    for(auto c : m_collections)
    {
      tpRemoveOne(c->m_unrefCallbacks, &m_unrefCallback);
      c->removeCallback(&m_callback);
    }
  }

  //################################################################################################
  void setCallback(const std::function<T>& callback)
  {
    m_callback = callback;
  }

  //################################################################################################
  void connect(CallbackCollection<T>& collection)
  {
    m_collections.push_back(&collection);
    collection.m_unrefCallbacks.push_back(&m_unrefCallback);
    collection.addCallback(&m_callback);
  }

  //################################################################################################
  R operator()(Args... args) const
  {
    if(m_callback)
      m_callback(args...);
    return; //Force void
  }

  private:
  std::function<T> m_callback;
  std::vector<C> m_collections;
  std::function<void(C)> m_unrefCallback;
};

}

#endif
