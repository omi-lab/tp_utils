#ifndef tp_utils_AbstractCrossThreadCallback_h
#define tp_utils_AbstractCrossThreadCallback_h

#include "tp_utils/CallbackCollection.h"

namespace tp_utils
{

//##################################################################################################
class TP_UTILS_SHARED_EXPORT AbstractCrossThreadCallback
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

protected:
  //################################################################################################
  void callback() const;

private:
  std::function<void()> m_callback;
  const std::function<void()> m_callFunctor;
};

//##################################################################################################
class TP_UTILS_SHARED_EXPORT AbstractCrossThreadCallbackFactory
{
  TP_NONCOPYABLE(AbstractCrossThreadCallbackFactory);
public:

  //################################################################################################
  AbstractCrossThreadCallbackFactory()=default;

  //################################################################################################
  virtual ~AbstractCrossThreadCallbackFactory();

  //################################################################################################
  [[nodiscard]] virtual AbstractCrossThreadCallback* produce(const std::function<void()>& callback) const = 0;
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
class TP_UTILS_SHARED_EXPORT PolledCrossThreadCallbackFactory: public AbstractCrossThreadCallbackFactory
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

}

#endif
