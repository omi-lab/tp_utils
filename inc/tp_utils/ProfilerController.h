#ifndef tp_utils_ProfilerController_h
#define tp_utils_ProfilerController_h

#include <string>
#include <vector>
#include <functional>

namespace tp_utils
{
class Profiler;
struct ProfilerResults;
//##################################################################################################
class ProfilerController
{

public:
  //##################################################################################################
  ProfilerController();

  //##################################################################################################
  ~ProfilerController();

  //##################################################################################################
  ProfilerController(const ProfilerController&) = delete;

  //##################################################################################################
  ProfilerController& operator=(const ProfilerController&) = delete;
  
  //##################################################################################################
  std::vector<std::string> getProfilerNames() const;

  //##################################################################################################
  void registerProfilerObserver(void* owner, std::function<void()> callBack);

  //##################################################################################################
  void deregisterProfilerObserver(void* owner);

  //##################################################################################################
  bool isRegistered(const std::string& profilerName);

  //##################################################################################################
  bool startRecording(const std::string& profilerName);

  //##################################################################################################
  bool stopRecording(const std::string& profilerName);

  //##################################################################################################
  void reset(const std::string& profilerName);

  //################################################################################################
  void viewResults(const std::string& profilerName, const std::function<void(const ProfilerResults&)>& closure);

protected:
  void registerProfiler(Profiler* profiler);
  void deregisterProfiler(Profiler* profiler);
  void onProfilerNameChanged(const std::string& oldName, const std::string& newName);
  bool isNameAvailable(const std::string& name);

private:
  struct Private;
  friend struct Private;
  friend class Profiler;
  Private* d;
};

#ifdef TP_ENABLE_PROFILING
//##################################################################################################
extern ProfilerController* globalProfilerController_;
#endif
}

#endif //tp_utils_Profiler_h
