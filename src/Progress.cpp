#include "tp_utils/Progress.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/RefCount.h"
#include "tp_utils/AbstractCrossThreadCallback.h"

namespace tp_utils
{

namespace
{
//##################################################################################################
struct ChildStep_lt
{
  std::vector<ProgressMessage> messages;
  Progress* childProgress{nullptr};

  float min{0.0f};
  float max{1.0f};
  float fraction{0.0f};
};
}

//##################################################################################################
struct Progress::Private
{
  TP_REF_COUNT_OBJECTS("tp_utils::Progress::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  Progress* q;
  bool printToConsole{false};
  size_t indentation{0};

  TPMutex mutex{TPM};
  std::string description;
  std::vector<ChildStep_lt> childSteps;
  bool shouldStop{false};

  std::unique_ptr<AbstractCrossThreadCallback> crossThreadCallback;
  std::function<bool()> poll;
  Progress* parent{nullptr};

  //################################################################################################
  Private(Progress* q_):
    q(q_)
  {

  }

  //################################################################################################
  ~Private()
  {
    for(const auto& childStep : childSteps)
      delete childStep.childProgress;
  }

  //################################################################################################
  void updateThis(const std::function<void(ChildStep_lt&)>& closure)
  {
    TP_MUTEX_LOCKER(mutex);

    float min=0.0f;
    bool create=false;

    if(childSteps.empty())
    {
      create = true;
      min=0.0f;
    }
    else if(childSteps.back().childProgress)
    {
      create = true;
      min=childSteps.back().max;
    }

    if(create)
    {
      auto& newChildStep = childSteps.emplace_back();
      newChildStep.min = min;
      newChildStep.max = 1.0f;
      newChildStep.fraction = min;
    }

    closure(childSteps.back());
  }

  //################################################################################################
  void checkPrint(const std::string& text)
  {
    if(!printToConsole)
      return;

    Progress* p=q;
    while(p->d->parent)
      p=p->d->parent;

    tpWarning() << "(" << int(p->progress()*100.1f) << "%) " << std::string(indentation, ' ') << text;
  }
};

//##################################################################################################
Progress::Progress(AbstractCrossThreadCallbackFactory* crossThreadCallbackFactory):
  d(new Private(this))
{
  d->crossThreadCallback.reset(crossThreadCallbackFactory->produce([&]{changed();}));
}

//##################################################################################################
Progress::Progress(const std::function<bool()>& poll):
  d(new Private(this))
{
  d->poll = poll;
}

//##################################################################################################
Progress::Progress(Progress* parent):
  d(new Private(this))
{
  d->parent = parent;
  d->indentation = parent->d->indentation + 2;
  d->printToConsole = parent->d->printToConsole;
}

//##################################################################################################
Progress::~Progress()
{
  delete d;
}

//##################################################################################################
void Progress::setPrintToConsole(bool printToConsole)
{
  d->printToConsole = printToConsole;
}

//##################################################################################################
void Progress::setProgress(float fraction)
{
  d->updateThis([&](ChildStep_lt& childStep)
  {
    childStep.fraction = fraction;
  });

  callChanged();
}

//##################################################################################################
void Progress::setProgress(float fraction, const std::string& description)
{
  d->updateThis([&](ChildStep_lt& childStep)
  {
    childStep.fraction = fraction;
    d->description = description;
  });

  callChanged();
}

//##################################################################################################
void Progress::setDescription(const std::string& description)
{
  d->updateThis([&](ChildStep_lt&)
  {
    d->description = description;
  });

  callChanged();
}

//##################################################################################################
std::string Progress::description() const
{
  TP_MUTEX_LOCKER(d->mutex);
  return d->description;
}

//##################################################################################################
float Progress::progress() const
{
  TP_MUTEX_LOCKER(d->mutex);
  if(d->childSteps.empty())
    return 0.0f;

  const ChildStep_lt& childStep = d->childSteps.back();

  if(childStep.childProgress)
  {
    float fraction = childStep.max - childStep.min;
    fraction *= childStep.childProgress->progress();
    fraction += childStep.min;

    return fraction;
  }

  return childStep.fraction;
}

//##################################################################################################
void Progress::stop(bool shouldStop)
{
  TP_MUTEX_LOCKER(d->mutex);
  d->shouldStop = shouldStop;

  for(const auto& childStep : d->childSteps)
    if(childStep.childProgress)
      childStep.childProgress->stop(shouldStop);
}

//##################################################################################################
bool Progress::poll()
{
  bool ok=true;

  if(d->parent)
    ok&=d->parent->poll();
  else if(d->poll)
    ok&=d->poll();

  TP_MUTEX_LOCKER(d->mutex);
  if(!ok)
    d->shouldStop = true;

  return ok&(!d->shouldStop);
}

//##################################################################################################
Progress* Progress::addChildStep(const std::string& message, float completeFraction)
{
  Progress* childProgress{nullptr};

  d->mutex.locked([&]
  {
    float min=0.0f;

    if(!d->childSteps.empty())
    {
      ChildStep_lt& childStep = d->childSteps.back();
      if(childStep.childProgress)
        min = childStep.max;
      else
        min = childStep.fraction;
    }

    ChildStep_lt& childStep = d->childSteps.emplace_back();
    childStep.messages.emplace_back(message, false, 0);
    childStep.childProgress = new Progress(this);
    childStep.min = min;
    childStep.max = completeFraction;
    childStep.fraction = 0.0f;

    childProgress = childStep.childProgress;
  });

  callChanged();  
  childProgress->d->checkPrint(message);

  return childProgress;
}

//##################################################################################################
void Progress::addMessage(const std::string& message)
{
  d->updateThis([&](ChildStep_lt& childStep)
  {
    childStep.messages.emplace_back(message, false, 0);
  });

  callChanged();
  d->checkPrint(message);
}

//##################################################################################################
void Progress::addError(const std::string& error)
{
  d->updateThis([&](ChildStep_lt& childStep)
  {
    childStep.messages.emplace_back(error, true, 0);
  });

  callChanged();
  d->checkPrint(error);
}

//##################################################################################################
std::vector<ProgressMessage> Progress::allMessages() const
{
  std::vector<ProgressMessage> messages;
  getAllMessages(0, messages);
  return messages;
}

//##################################################################################################
std::vector<ProgressMessage> Progress::errorMessages() const
{
  std::vector<ProgressMessage> messages;
  getErrors(0, messages);
  return messages;
}

//##################################################################################################
std::string Progress::compileErrors() const
{
  std::string result;
  for(const auto& message : errorMessages())
    result += message.message + '\n';
  return result;
}

//##################################################################################################
std::vector<Progress*> Progress::childSteps() const
{
  TP_MUTEX_LOCKER(d->mutex);
  std::vector<Progress*> childSteps;
  childSteps.reserve(d->childSteps.size());
  for(const auto& childStep : d->childSteps)
    if(childStep.childProgress)
      childSteps.push_back(childStep.childProgress);

  return childSteps;
}

//##################################################################################################
bool Progress::shouldStop() const
{
  if(d->parent)
    return d->parent->shouldStop();

  TP_MUTEX_LOCKER(d->mutex);
  return d->shouldStop;
}

//##################################################################################################
void Progress::callChanged()
{
  if(d->parent)
    d->parent->callChanged();
  else if(d->crossThreadCallback)
    d->crossThreadCallback->call();
  else
    changed();
}

//##################################################################################################
void Progress::getAllMessages(size_t indentation, std::vector<ProgressMessage>& messages) const
{
  TP_MUTEX_LOCKER(d->mutex);
  for(const auto& childStep : d->childSteps)
  {
    for(const auto& message : childStep.messages)
      messages.emplace_back(message).indentation = indentation;
    if(childStep.childProgress)
      childStep.childProgress->getAllMessages(indentation+1, messages);
  }
}

//##################################################################################################
bool Progress::getErrors(size_t indentation, std::vector<ProgressMessage>& messages) const
{
  TP_MUTEX_LOCKER(d->mutex);
  bool error=false;
  for(const auto& childStep : d->childSteps)
  {
    for(const auto& message : childStep.messages)
    {
      if(message.error)
      {
        messages.emplace_back(message).indentation = indentation;
        error = true;
      }
    }
    if(childStep.childProgress)
      error |= childStep.childProgress->getErrors(indentation+1, messages);
  }

  return error;
}

}
