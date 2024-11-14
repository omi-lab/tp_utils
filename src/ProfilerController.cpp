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

  //################################################################################################
  template<typename T>
  void iterateProfilers(const T& closure)
  {
    for(auto i=profilers.begin(); i!=profilers.end(); )
    {
      if(auto p=i->lock(); p)
      {
        closure(p);
        ++i;
      }
      else
        i=profilers.erase(i);
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
std::vector<const Profiler*> ProfilerController::profilers() const
{
  std::vector<const Profiler*> profilers;
  profilers.reserve(d->profilers.size());
  d->iterateProfilers([&](auto p){profilers.push_back(p.get());});
  return profilers;
}

//##################################################################################################
void ProfilerController::updateProfilers(const std::function<void(std::vector<std::weak_ptr<Profiler>>&)>& closure) const
{
  closure(d->profilers);
}

//##################################################################################################
std::shared_ptr<Profiler> ProfilerController::profiler(const StringID& id)
{
  assert(id.isValid());

  std::shared_ptr<Profiler> profiler;
  d->iterateProfilers([&](auto p)
  {
    if(p->id == id)
      profiler = p;
  });

  for(const auto& profiler : d->profilers)
    if(auto p = profiler.lock(); p->id == id)
      return p;

  if(!profiler)
  {
    profiler.reset(new Profiler(this, id));
    d->profilers.emplace_back(profiler);
  }

  changed();
  return profiler;
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
#else

// Silence warning for empty .o file
int profilerController_cpp()
{
  return 0;
}


#endif
