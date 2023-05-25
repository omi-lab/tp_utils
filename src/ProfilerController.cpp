#ifdef TP_ENABLE_PROFILING

#include "tp_utils/ProfilerController.h"
#include "tp_utils/Profiler.h"

#include <unordered_map>

namespace tp_utils
{

//##################################################################################################
ProfilerController* globalProfilerController()
{
  static std::unique_ptr<ProfilerController> globalProfilerController;
  if(!globalProfilerController)
    globalProfilerController = std::make_unique<ProfilerController>();
  return globalProfilerController.get();
}

//##################################################################################################
struct ProfilerController::Private
{
  std::vector<std::weak_ptr<Profiler>> profilers;
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
std::vector<std::shared_ptr<Profiler>> ProfilerController::profilers() const
{
  std::vector<std::shared_ptr<Profiler>> profilers;
  profilers.reserve(d->profilers.size());
  for(const auto& profiler : d->profilers)
    profilers.push_back(profiler.lock());
  return profilers;
}

//##################################################################################################
std::shared_ptr<Profiler> ProfilerController::profiler(const StringID& id)
{
  assert(id.isValid());

  for(const auto& profiler : d->profilers)
    if(auto p = profiler.lock(); p->id == id)
      return p;

  std::shared_ptr<Profiler> p{new Profiler(this, id)};
  d->profilers.emplace_back(p);
  changed();
  return p;
}

//##################################################################################################
void ProfilerController::profilerDeleted(Profiler* profiler)
{
  for(auto i=d->profilers.begin(); i!=d->profilers.end(); )
  {
    if(i->expired() || i->lock().get() == profiler)
      i = d->profilers.erase(i);
    else
      ++i;
  }

  changed();
}

}

#endif
