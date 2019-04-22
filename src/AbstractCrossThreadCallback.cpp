#include "tp_utils/AbstractCrossThreadCallback.h"
#include "tp_utils/MutexUtils.h"

namespace tp_utils
{

//##################################################################################################
AbstractCrossThreadCallback::AbstractCrossThreadCallback(const std::function<void()>& callback):
  m_callback(callback),
  m_callFunctor([this]{call();})
{

}

//##################################################################################################
const std::function<void()>* AbstractCrossThreadCallback::callFunctor() const
{
  return &m_callFunctor;
}

//##################################################################################################
void AbstractCrossThreadCallback::callback() const
{
  m_callback();
}

//##################################################################################################
AbstractCrossThreadCallbackFactory::~AbstractCrossThreadCallbackFactory()
{

}

namespace
{
//##################################################################################################
class PolledCrossThreadCallback: public AbstractCrossThreadCallback
{
public:
  //################################################################################################
  PolledCrossThreadCallback(const std::function<void()>& callback_):
    AbstractCrossThreadCallback(callback_)
  {
    poll.setCallback([this]()
    {
      size_t c = m_mutex.locked(TPMc[&]{return std::exchange(m_count, 0);});
      for(; c; c--)
        callback();
    });
  }

  //################################################################################################
  void call() override
  {
    TP_MUTEX_LOCKER(m_mutex);
    m_count++;
  }

  Callback<void()> poll;

private:
  TPMutex m_mutex{TPM};
  size_t m_count{0};
};

}

//##################################################################################################
PolledCrossThreadCallbackFactory::PolledCrossThreadCallbackFactory()
{
  poll.setCallback([&]{m_pollAll();});
}

//##################################################################################################
AbstractCrossThreadCallback* PolledCrossThreadCallbackFactory::produce(const std::function<void()>& callback) const
{
  auto c = new PolledCrossThreadCallback(callback);
  c->poll.connect(m_pollAll);
  return c;
}

}
