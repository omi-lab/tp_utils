#include "tp_utils/AbstractTimerCallback.h"
#include "tp_utils/TimeUtils.h"

namespace tp_utils
{

//##################################################################################################
AbstractTimerCallback::AbstractTimerCallback(const std::function<void()>& callback, int64_t timeOutMS):
  m_callback(callback),
  m_timeOutMS(timeOutMS)
{

}

//##################################################################################################
AbstractTimerCallback::~AbstractTimerCallback()=default;

//##################################################################################################
void AbstractTimerCallback::setTimeOutMS(int64_t timeOutMS)
{
  m_timeOutMS = timeOutMS;
}

//##################################################################################################
int64_t AbstractTimerCallback::timeOutMS()
{
  return m_timeOutMS;
}

//##################################################################################################
void AbstractTimerCallback::callback() const
{
  m_callback();
}

//##################################################################################################
AbstractTimerCallbackFactory::~AbstractTimerCallbackFactory()
{

}

namespace
{
//##################################################################################################
class PolledTimerCallback: public AbstractTimerCallback
{
public:
  //################################################################################################
  PolledTimerCallback(const std::function<void()>& callback_, int64_t timeOutMS):
    AbstractTimerCallback(callback_, timeOutMS),
    m_nextCall(currentTimeMS() + timeOutMS)
  {
    poll.setCallback([&]()
    {
      if(currentTimeMS()>=m_nextCall)
      {
        m_nextCall = currentTimeMS() + this->timeOutMS();
        callback();
      }
    });
  }

  //################################################################################################
  void setTimeOutMS(int64_t timeOutMS)
  {
    AbstractTimerCallback::setTimeOutMS(timeOutMS);
    m_nextCall = currentTimeMS() + timeOutMS;
  }

  Callback<void()> poll;

private:
  int64_t m_nextCall;
};

}

//##################################################################################################
PolledTimerCallbackFactory::PolledTimerCallbackFactory()
{
  poll.setCallback([&]{m_pollAll();});
}

//##################################################################################################
AbstractTimerCallback* PolledTimerCallbackFactory::produce(const std::function<void()>& callback, int64_t timeOutMS) const
{
  auto c = new PolledTimerCallback(callback, timeOutMS);
  c->poll.connect(m_pollAll);
  return c;
}

}
