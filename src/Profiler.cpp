#include "tp_utils/Profiler.h"
#include "tp_utils/Progress.h"
#include "tp_utils/TimeUtils.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/ProfilerController.h"

#include <memory>
#include <sstream>
#include <stack>

namespace tp_utils
{

//##################################################################################################
struct Profiler::Private
{
  Private()
  {
  }
  
  std::string name;
  std::unique_ptr<RAMProgressStore> progressStore{std::make_unique<RAMProgressStore>()};
  bool recording{false};

  struct SummaryGenerator
  {
    std::string label;
    std::function<double(const std::vector<ProgressEvent>&)> calcfun{nullptr};
    std::function<std::string(double)> printer{nullptr};
  };

  std::vector<SummaryGenerator> summaryGenerators;
  std::stack<tp_utils::ProgressEvent> eventStack;
  ProfilerResults results;
};

//##################################################################################################
Profiler::Profiler(const std::string& name):
  d(new Private())
{
  setName(name.empty() ? "Profiler" : name);
#ifdef TP_ENABLE_PROFILING
  tp_utils::globalProfilerController_->registerProfiler(this);
#endif
}

//##################################################################################################
Profiler::~Profiler()
{
#ifdef TP_ENABLE_PROFILING
tp_utils::globalProfilerController_->deregisterProfiler(this);
#endif
  delete d;
}

//##################################################################################################
std::string Profiler::name() const
{
  return d->name;
}

//##################################################################################################
std::string Profiler::setName(const std::string& name_)
{
  if(d->recording || name_ == d->name)
    return d->name;

  std::string newName = name_;

#ifdef TP_ENABLE_PROFILING
  std::string oldName = d->name;

  bool nameIsValid = tp_utils::globalProfilerController_->isNameAvailable(newName);
  size_t counter = 0;
  while(!nameIsValid)
  {
    std::stringstream ss;
    ss << name_ << counter++;
    newName = ss.str();
    nameIsValid = tp_utils::globalProfilerController_->isNameAvailable(newName);
  }
  
  tp_utils::globalProfilerController_->onProfilerNameChanged(oldName, newName);
#endif

  d->name = newName;
  return name_;
}

//##################################################################################################
bool Profiler::isRecording() const
{
  return d->recording;
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

//################################################################################################
void Profiler::viewResults(const std::function<void(const ProfilerResults&)>& closure)
{
  d->progressStore->viewProgressEvents([&](const auto& p){
    d->results.events = p;

    d->results.summaries.clear();
    for(auto summaryGene : d->summaryGenerators)
    {
        auto& summaryItem = d->results.summaries.emplace_back();
        summaryItem.label = summaryGene.label;
        summaryItem.value = summaryGene.calcfun(d->results.events);
        if(summaryGene.printer)
        {
            summaryItem.formattedOutput = summaryGene.printer(summaryItem.value);
        }
        else
        {
            std::stringstream ss;
            ss << summaryItem.label << ": "<<summaryItem.value;
            summaryItem.formattedOutput = ss.str();
        }
    }
    closure(d->results);
  });


}

//################################################################################################
void Profiler::addSummaryGenerator(const std::string& label, const std::function<double(const std::vector<ProgressEvent>&)>& calcFunc, const std::function<std::string(double)>& printer)
{
  auto& summaryGenerator = d->summaryGenerators.emplace_back();
  summaryGenerator.label = label;
  summaryGenerator.calcfun = calcFunc;
  summaryGenerator.printer = printer;
}

//##################################################################################################
void Profiler::reset()
{
  d->progressStore.reset(new RAMProgressStore());
}

//##################################################################################################
bool Profiler::record()
{
  if(!d->recording)
  {
    d->recording = true;
    return true;
  }
  return false;
}

//##################################################################################################
bool Profiler::stop()
{
  d->recording = false;
  return true;
}

}
