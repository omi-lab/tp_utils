#ifndef tp_utils_ProfilerController_h
#define tp_utils_ProfilerController_h

#ifdef TP_ENABLE_PROFILING

#include "tp_utils/StringID.h"
#include "tp_utils/CallbackCollection.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace tp_utils
{
class Profiler;
struct ProfilerResults;
class ProfilerController;

//##################################################################################################
ProfilerController* globalProfilerController();

//##################################################################################################
class ProfilerController
{
  TP_NONCOPYABLE(ProfilerController);
public:
  //################################################################################################
  ProfilerController();

  //################################################################################################
  ~ProfilerController();
  
  //################################################################################################
  std::vector<std::shared_ptr<Profiler>> profilers() const;

  //################################################################################################
  std::shared_ptr<Profiler> profiler(const StringID& id);

  //################################################################################################
  //! Emitted when the list of profilers changes
  CallbackCollection<void()> changed;

protected:
  //################################################################################################
  void profilerDeleted(Profiler* profiler);

private:
  struct Private;
  friend struct Private;
  friend class Profiler;
  Private* d;
};

}

#endif
#endif
