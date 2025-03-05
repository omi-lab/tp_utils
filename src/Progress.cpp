#include "tp_utils/Progress.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/RefCount.h"
#include "tp_utils/AbstractCrossThreadCallback.h"
#include "tp_utils/TimeUtils.h"
#include "tp_utils/JSONUtils.h"
#include "tp_utils/detail/log_stats/virtual_memory.h"

#include "lib_platform/Format.h"

#include <optional>

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
void ProgressEvent::saveState(nlohmann::json& j) const
{
  j["id"] = id;
  j["parentId"] = parentId;
  j["name"] = name;

  j["start"] = start;
  j["end"] = (end==0)?currentTimeMS():end;
  j["fraction"] = fraction;
  j["color"] = color.toString();
  j["active"] = false;
}

//##################################################################################################
void ProgressEvent::loadState(const nlohmann::json& j)
{
  id = TPJSONSizeT(j, "id");
  parentId = TPJSONSizeT(j, "parentId");
  name = TPJSONString(j, "name");

  start = TPJSONInt64T(j, "start");
  end = TPJSONInt64T(j, "end");
  fraction = TPJSONFloat(j, "fraction");
  color = TPPixel(TPJSONString(j, "color"));
  active = TPJSONBool(j, "active");
}

//##################################################################################################
AbstractProgressStore::AbstractProgressStore()=default;

//##################################################################################################
AbstractProgressStore::~AbstractProgressStore()=default;

//##################################################################################################
struct RAMProgressStore::Private
{
  TPMutex mutex{TPM};
  std::vector<ProgressEvent> progressEvents;
};

//##################################################################################################
RAMProgressStore::RAMProgressStore():
  d(new Private())
{
  ProgressEvent& progressEvent = d->progressEvents.emplace_back();
  progressEvent.store = this;
  progressEvent.id = 0;
  progressEvent.name = "Root";
  progressEvent.start = currentTimeMS();
  progressEvent.end = currentTimeMS();
}

//##################################################################################################
RAMProgressStore::~RAMProgressStore()
{
  delete d;
}

//##################################################################################################
void RAMProgressStore::initProgressEvent(ProgressEvent& progressEvent)
{
  TP_MUTEX_LOCKER(d->mutex);
  progressEvent.store = this;
  progressEvent.id = d->progressEvents.size();
  d->progressEvents.push_back(progressEvent);
}

//##################################################################################################
void RAMProgressStore::updateProgressEvent(const ProgressEvent& progressEvent)
{
  TP_MUTEX_LOCKER(d->mutex);
  assert(progressEvent.store == this);
  if(progressEvent.id<d->progressEvents.size())
    d->progressEvents[progressEvent.id] = progressEvent;
}

//##################################################################################################
void RAMProgressStore::viewProgressEvents(const std::function<void(const std::vector<ProgressEvent>&)>& closure)
{
  TP_MUTEX_LOCKER(d->mutex);
  closure(d->progressEvents);
}

//##################################################################################################
void RAMProgressStore::saveState(nlohmann::json& j) const
{
  j = nlohmann::json::array();

  {
    TP_MUTEX_LOCKER(d->mutex);
    j.get_ptr<nlohmann::json::array_t*>()->reserve(d->progressEvents.size());
    for(const auto& progressEvent : d->progressEvents)
    {
      j.emplace_back();
      progressEvent.saveState(j.back());
    }
  }
}

//##################################################################################################
std::vector<ProgressEvent> RAMProgressStore::loadState(const nlohmann::json& j)
{
  std::vector<ProgressEvent> progressEvents;
  if(!j.is_array())
    return progressEvents;

  progressEvents.reserve(j.size());

  for(const auto& jj : j)
    progressEvents.emplace_back().loadState(jj);

  return progressEvents;
}

////##################################################################################################
//std::string RAMProgressStore::saveState() const
//{
//  std::string result;

//  d->progressEvents.front().end = currentTimeMS();

//  for(const auto& progressEvent : d->progressEvents)
//  {
//    result += std::to_string(progressEvent.id) + " ";
//    result += std::to_string(progressEvent.parentId) + " ";
//    result += progressEvent.name + " ";
//    result += std::to_string(progressEvent.color.r) + ",";
//    result += std::to_string(progressEvent.color.g) + ",";
//    result += std::to_string(progressEvent.color.b) + " ";
//    result += std::to_string(progressEvent.start) + " ";
//    result += std::to_string(progressEvent.end) + "\n";
//  }

//  return result;
//}

//##################################################################################################
AbstractProgressStore* globalProgressStore_{nullptr};

//##################################################################################################
struct Progress::Private
{
  TP_REF_COUNT_OBJECTS("tp_utils::Progress::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  Q* q;
  bool printToConsole{false};
  size_t indentation{0};

  TPMutex mutex{TPM};
  std::string description;
  std::vector<ChildStep_lt> childSteps;
  bool shouldStop{false};

  TPCrossThreadCallback crossThreadCallback;
  std::function<bool()> poll;
  Progress* parent{nullptr};

  AbstractProgressStore* progressStore{nullptr};
  std::optional<ProgressEvent> progressEvent;

  //################################################################################################
  Private(Q* q_,
          AbstractProgressStore* progressStore_,
          Progress* parent_,
          const std::string& message):
    q(q_),
    parent(parent_),
    progressStore(progressStore_?progressStore_:globalProgressStore_)
  {
    if(progressStore)
    {
      progressEvent = ProgressEvent();
      progressEvent->name = message;

      if(parent)
        parent->viewProgressEvent([&](const auto& p){progressEvent->parentId = p.id;});

      progressEvent->start = currentTimeMS();
      progressEvent->end = currentTimeMS();

      progressStore->initProgressEvent(*progressEvent);
    }
  }

  //################################################################################################
  ~Private()
  {
    for(const auto& childStep : childSteps)
      delete childStep.childProgress;

    if(progressEvent)
    {
      progressEvent->active = false;
      progressStore->updateProgressEvent(*progressEvent);
    }
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

    tp_utils::detail::VirtualMemory vm;

    tpWarning() << std::format("({}MB, {}%) {}{}",
                               int(vm.VmHWM/1024),
                               int(p->progress()*100.1f),
                               std::string(indentation, ' '), text);
  }

  //################################################################################################
  template<typename T>
  void updateProgressEvent(const T& closure)
  {
    if(progressEvent)
    {
      closure(*progressEvent);
      progressStore->updateProgressEvent(*progressEvent);
    }
  }

  //################################################################################################
  void updateProgressEventEnd(ChildStep_lt& childStep, bool active)
  {
    updateProgressEvent([&](ProgressEvent& progressEvent)
    {
      progressEvent.fraction = childStep.max - childStep.min;
      progressEvent.fraction *= childStep.fraction;
      progressEvent.fraction += childStep.min;
      progressEvent.end = currentTimeMS();
      progressEvent.active = active;
    });
  }
};

//##################################################################################################
Progress::Progress(AbstractCrossThreadCallbackFactory* crossThreadCallbackFactory,
                   const std::string& message,
                   AbstractProgressStore* progressStore):
  d(new Private(this, progressStore, nullptr, message))
{
  d->crossThreadCallback = crossThreadCallbackFactory->produceP([&]{changed();});
}

//##################################################################################################
Progress::Progress(const std::function<bool()>& poll,
                   const std::string& message,
                   AbstractProgressStore* progressStore):
  d(new Private(this, progressStore, nullptr, message))
{
  d->poll = poll;
}

//##################################################################################################
Progress::Progress(Progress* parent, const std::string& message):
  d(new Private(this, parent->progressStore(), parent, message))
{
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
    d->updateProgressEventEnd(childStep, fraction<=0.99999f);
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
    d->updateProgressEventEnd(childStep, fraction<=0.99999f);
  });

  d->checkPrint(description);
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
AbstractProgressStore* Progress::progressStore() const
{
  return d->progressStore;
}

//##################################################################################################
void Progress::updateProgressEvent(const std::function<void(ProgressEvent&)>& closure) const
{
  d->updateProgressEvent(closure);
}

//##################################################################################################
void Progress::viewProgressEvent(const std::function<void(const ProgressEvent&)>& closure) const
{
  if(d->progressEvent)
    closure(*d->progressEvent);
}

//##################################################################################################
void Progress::copyChildSteps(Progress* progress, const std::string& message, float completeFraction)
{  
  auto dstChildStep = addChildStep(message, completeFraction);

  for(const auto& childStep : progress->d->childSteps)
  {
    dstChildStep->setProgress(childStep.fraction);

    for(size_t m=0; m<childStep.messages.size(); m++)
    {
      if(m==(childStep.messages.size()-1) && childStep.childProgress)
        break;

      const auto& message = childStep.messages.at(m);

      if(message.error)
        dstChildStep->addError(message.message);
      else
        dstChildStep->addMessage(message.message);
    }

    if(childStep.childProgress && !childStep.messages.empty())
      dstChildStep->copyChildSteps(childStep.childProgress, childStep.messages.back().message, 1.0f);
  }
}

//##################################################################################################
Progress* Progress::addChildStep(const std::string& message, float completeFraction)
{
  Progress* childProgress{nullptr};

  d->mutex.locked(TPMc [&]
  {
    float min=0.0f;

    if(!d->childSteps.empty())
    {
      ChildStep_lt& childStep = d->childSteps.back();
      if(childStep.childProgress)
        min = childStep.max;
      else
        min = childStep.fraction;

      d->updateProgressEventEnd(childStep, false);
    }

    ChildStep_lt& childStep = d->childSteps.emplace_back();
    childStep.messages.emplace_back(message, false, 0);
    childStep.childProgress = new Progress(this, message);
    childStep.min = min;
    childStep.max = completeFraction;
    childStep.fraction = 0.0f;

    childProgress = childStep.childProgress;

    d->updateProgressEventEnd(childStep, true);
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
    d->updateProgressEventEnd(childStep, false);
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

//##################################################################################################
struct ParrallelProgress::Private
{
  struct Child_lt
  {
    tp_utils::RAMProgressStore* store{nullptr};
    Progress* progress;
    std::string message;
  };

  Progress* progress;
  TPMutex mutex{TPM};
  std::vector<Child_lt> childSteps;

  //################################################################################################
  Private(Progress* progress_):
    progress(progress_)
  {

  }
};

//##################################################################################################
ParrallelProgress::ParrallelProgress(Progress* progress):
  d(new Private(progress))
{

}

//##################################################################################################
ParrallelProgress::~ParrallelProgress()
{
  for(size_t s=0; s<d->childSteps.size(); s++)
  {
    const auto& childStep = d->childSteps.at(s);
    d->progress->copyChildSteps(childStep.progress, childStep.message, float(s+1)/float(d->childSteps.size()));
    delete childStep.progress;
    delete childStep.store;
  }
  d->progress->setProgress(1.0f);
  d->progress->addMessage("Done.");
  delete d;
}

//##################################################################################################
Progress* ParrallelProgress::addChildStep(const std::string& message)
{
  TP_MUTEX_LOCKER(d->mutex);
  auto& childStep = d->childSteps.emplace_back();
  childStep.store = new tp_utils::RAMProgressStore();
  childStep.progress = new Progress([&]{return !d->progress->shouldStop();}, message);
  childStep.message = message;
  return childStep.progress;
}

}
