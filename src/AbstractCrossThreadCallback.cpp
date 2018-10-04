#include "tp_utils/AbstractCrossThreadCallback.h"

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

}
