#ifdef TP_ENABLE_PROFILING

#include "tp_utils/Profiler.h"
#include "tp_utils/Progress.h"
#include "tp_utils/TimeUtils.h"
#include "tp_utils/ProfilerController.h"

#include <memory>
#include <sstream>
#include <stack>

namespace tp_utils
{

//##################################################################################################
struct Profiler::Private
{
  ProfilerController* controller;
  std::string name;
  std::unique_ptr<RAMProgressStore> progressStore{std::make_unique<RAMProgressStore>()};

  bool recording{false};

  std::vector<SummaryGenerator> summaryGenerators;
  std::stack<tp_utils::ProgressEvent> eventStack;

  Private(ProfilerController* controller_):
    controller(controller_)
  {
  }
};

//##################################################################################################
Profiler::Profiler(ProfilerController* controller, const StringID& id_):
  d(new Private(controller)),
  id(id_)
{
}

//##################################################################################################
Profiler::~Profiler()
{
  d->controller->profilerDeleted(this);
  delete d;
}

//##################################################################################################
std::string Profiler::name() const
{
  return d->name;
}

//##################################################################################################
void Profiler::setName(const std::string& name)
{
  d->name = name;

  d->controller->updateProfilers([&](std::vector<std::weak_ptr<Profiler>>& profilers)
  {
    auto containsName = [&](const std::string& n)
    {
      for(const auto& profiler : profilers)
        if(auto p=profiler.lock(); p)
          if(p->name()==n)
            return true;

      return false;
    };

    auto toString = [](size_t i)
    {
      return ' ' + tp_utils::fixedWidthKeepRight(std::to_string(i), 3, '0');
    };

    for(const auto& profiler : profilers)
    {
      if(auto p=profiler.lock(); p)
      {
        if(p.get()!=this && p->name() == name)
        {
          std::string n=p->name();
          if(p->name().size()>3)
          {
            if(std::isdigit(n.at(n.size()-1)) &&
               std::isdigit(n.at(n.size()-2)) &&
               std::isdigit(n.at(n.size()-3)) &&
               n.at(n.size()-4) == ' ')
              n.resize(p->name().size()-4);
          }

          if(containsName(n))
          {
            size_t i=1;
            for(; containsName(n + toString(i)); i++);
            n = n + toString(i);
          }

          p->d->name = n;
        }
      }
    }
  });
  d->controller->changed();
}

//##################################################################################################
void Profiler::rangePush(const std::string& label, TPPixel colour)
{
  if(!d->recording)
    return;

  size_t parentId = 0;
  if(!d->eventStack.empty())
    parentId = d->eventStack.top().id;

  auto& progressEvent = d->eventStack.emplace();
  progressEvent.parentId = parentId;
  progressEvent.name = label;
  progressEvent.start = tp_utils::currentTimeMS();
  progressEvent.end = tp_utils::currentTimeMS();
  progressEvent.active = true;
  progressEvent.color = colour;
  d->progressStore->initProgressEvent(progressEvent);
}

//##################################################################################################
void Profiler::rangePop()
{
  if(!d->recording)
    return;

  auto& progressEvent = d->eventStack.top();
  progressEvent.end = tp_utils::currentTimeMS();
  progressEvent.active = false;
  d->progressStore->updateProgressEvent(progressEvent);
  d->eventStack.pop();
}

//##################################################################################################
void Profiler::viewProgressEvents(const std::function<void(const std::vector<ProgressEvent>&)>& closure) const
{
  d->progressStore->viewProgressEvents(closure);
}

//##################################################################################################
std::vector<std::pair<std::string, std::string>> Profiler::summaries() const
{
  std::vector<std::pair<std::string, std::string>> summaries;
  for(const auto& summaryGenerator : d->summaryGenerators)
    summaryGenerator(*this, summaries);
  return summaries;
}

//##################################################################################################
void Profiler::addSummaryGenerator(const SummaryGenerator& summaryGenerator)
{
  d->summaryGenerators.push_back(summaryGenerator);
}

//##################################################################################################
void Profiler::clearSummaryGenerators()
{
  d->summaryGenerators.clear();
}

//##################################################################################################
void Profiler::startRecording()
{
  d->progressStore.reset(new RAMProgressStore());
  d->recording = true;
  d->controller->changed();
}

//##################################################################################################
void Profiler::stopRecording()
{
  if(d->recording)
  {
    d->recording = false;
    d->controller->changed();
  }
}

//##################################################################################################
void Profiler::saveState(nlohmann::json& j) const
{
  d->progressStore->saveState(j);
}

//##################################################################################################
bool Profiler::isRecording() const
{
  return d->recording;
}

}
#else

// Silence warning for empty .o file
int profiler_cpp()
{
  return 0;
}

#endif
