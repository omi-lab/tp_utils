#ifndef tp_utils_AbstractCrossThreadCallback_h
#define tp_utils_AbstractCrossThreadCallback_h

#include "tp_utils/CallbackCollection.h"

#include <thread>
namespace tp_utils
{

//##################################################################################################
class TP_UTILS_EXPORT AbstractCrossThreadCallback
{
  TP_NONCOPYABLE(AbstractCrossThreadCallback);
public:
  //################################################################################################
  AbstractCrossThreadCallback(std::function<void()> callback);

  //################################################################################################
  virtual ~AbstractCrossThreadCallback()=default;

  //################################################################################################
  virtual void call() = 0;

  //################################################################################################
  const std::function<void()>* callFunctor() const;

  //################################################################################################
  void operator()()
  {
    call();
  }

protected:
  //################################################################################################
  void callback() const;

private:
  std::function<void()> m_callback;
  const std::function<void()> m_callFunctor;
};
}

//##################################################################################################
typedef std::unique_ptr<tp_utils::AbstractCrossThreadCallback> TPCrossThreadCallback;

namespace tp_utils
{

//##################################################################################################
class TP_UTILS_EXPORT AbstractCrossThreadCallbackFactory
{
  TP_NONCOPYABLE(AbstractCrossThreadCallbackFactory);
  std::thread::id m_mainThreadID{std::this_thread::get_id()};
public:

  //################################################################################################
  AbstractCrossThreadCallbackFactory()=default;

  //################################################################################################
  virtual ~AbstractCrossThreadCallbackFactory();

  //################################################################################################
  [[nodiscard]] virtual AbstractCrossThreadCallback* produce(const std::function<void()>& callback) const = 0;

  //################################################################################################
  [[nodiscard]] TPCrossThreadCallback produceP(const std::function<void()>& callback) const
  {
    return std::unique_ptr<tp_utils::AbstractCrossThreadCallback>(produce(callback));
  }

  //################################################################################################
  bool sameThread()
  {
    return m_mainThreadID == std::this_thread::get_id();
  }
};

//##################################################################################################
template<typename T>
class CrossThreadCallbackFactoryTemplate : public AbstractCrossThreadCallbackFactory
{
  AbstractCrossThreadCallback* produce(const std::function<void()>& callback) const override
  {
    return new T(callback);
  }
};

//##################################################################################################
class TP_UTILS_EXPORT PolledCrossThreadCallbackFactory: public AbstractCrossThreadCallbackFactory
{
public:
  //################################################################################################
  PolledCrossThreadCallbackFactory();

  //################################################################################################
  AbstractCrossThreadCallback* produce(const std::function<void()>& callback) const override;

  //################################################################################################
  Callback<void()> poll;

private:
  mutable CallbackCollection<void()> m_pollAll;
};

//##################################################################################################
void blockingCrossThreadCall(AbstractCrossThreadCallbackFactory* factory, const std::function<void()>& callback);

}


#endif
