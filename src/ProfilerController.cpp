#include "tp_utils/ProfilerController.h"
#include "tp_utils/Profiler.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/MutexUtils.h"


#include <unordered_map>

namespace tp_utils
{

#ifdef TP_ENABLE_PROFILING
//##################################################################################################
ProfilerController* globalProfilerController_{nullptr};
#endif

//##################################################################################################
struct ProfilerController::Private
{
  std::unordered_map<std::string, Profiler*> profilers;
  std::unordered_map<void*, std::function<void()>> changeObservers;

  void notifyChangeObservers()
  {
    for(auto it = changeObservers.begin(); it != changeObservers.end(); ++it) {
      it->second();
    }
  }
};

//##################################################################################################
ProfilerController::ProfilerController():
  d(new Private())
{
}

//##################################################################################################
ProfilerController::~ProfilerController()
{
  delete d;
}

//##################################################################################################
void ProfilerController::registerProfiler(Profiler* profiler)
{
  assert(d->profilers.find(profiler->name()) == d->profilers.end());
  d->profilers[profiler->name()] = profiler;
  d->notifyChangeObservers();
}

//##################################################################################################
void ProfilerController::deregisterProfiler(Profiler* profiler)
{
  d->profilers[profiler->name()]->stop();
  d->profilers.erase(profiler->name());
  d->notifyChangeObservers();
}

//##################################################################################################
void ProfilerController::onProfilerNameChanged(const std::string& oldName, const std::string& newName)
{
  assert(d->profilers.find(newName) == d->profilers.end());
  d->profilers[newName] = d->profilers[oldName];
  d->profilers.erase(oldName);
  d->notifyChangeObservers();
}

//##################################################################################################
bool ProfilerController::isNameAvailable(const std::string& name)
{
  return (d->profilers.find(name) == d->profilers.end());
}

//##################################################################################################
std::vector<std::string> ProfilerController::getProfilerNames() const
{
  std::vector<std::string> out;
  for(auto it = d->profilers.begin(); it != d->profilers.end(); ++it) {
    out.push_back(it->first);
  }
  return out;
}

//##################################################################################################
void ProfilerController::registerProfilerObserver(void* owner, std::function<void()> callBack)
{
  assert(d->changeObservers.find(owner) == d->changeObservers.end());
  d->changeObservers[owner] = callBack;
  d->changeObservers[owner]();
}

//##################################################################################################
void ProfilerController::deregisterProfilerObserver(void* owner)
{
  d->changeObservers.erase(owner);
}

//##################################################################################################
bool ProfilerController::isRegistered(const std::string& profilerName)
{
  return (d->profilers.find(profilerName) != d->profilers.end());
}

//##################################################################################################
bool ProfilerController::startRecording(const std::string& profilerName)
{
  if(d->profilers.find(profilerName) == d->profilers.end())
    return false;
  return d->profilers[profilerName]->record();
}

//##################################################################################################
bool ProfilerController::stopRecording(const std::string& profilerName)
{
  if(d->profilers.find(profilerName) == d->profilers.end())
    return false;
  return d->profilers[profilerName]->stop();
}

//##################################################################################################
void ProfilerController::reset(const std::string& profilerName)
{
  if(d->profilers.find(profilerName) == d->profilers.end())
    return;
  d->profilers[profilerName]->reset();
}

void ProfilerController::viewResults(const std::string& profilerName, const std::function<void(const ProfilerResults&)>& closure)
{
  if(d->profilers.find(profilerName) == d->profilers.end())
    return;
  d->profilers[profilerName]->viewResults(closure);
}


}
