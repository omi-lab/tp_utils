#ifndef tp_utils_Profiler_h
#define tp_utils_Profiler_h

#include "tp_utils/TPPixel.h"

#include <string>


namespace tp_utils
{
class ProfilerController;
struct ProgressEvent;

//##################################################################################################
struct ProfilerSummaryItem
{
  std::string label;
  double value;
  std::string formattedOutput;
};

//##################################################################################################
struct ProfilerResults
{
 std::vector<ProgressEvent> events;
 std::vector<ProfilerSummaryItem> summaries;
};

//##################################################################################################
class Profiler
{ 

public:
  //##################################################################################################
  Profiler() = delete;

  //##################################################################################################
  Profiler(const std::string& name);

  //##################################################################################################
  virtual ~Profiler();

  //##################################################################################################
  Profiler(const Profiler&) = delete;

  //##################################################################################################
  Profiler& operator=(const Profiler&) = delete;

  //##################################################################################################
  std::string name() const;

  //##################################################################################################
  std::string setName(const std::string& name_);

  //##################################################################################################
  bool isRecording() const;

  //##################################################################################################
  void rangePush(const std::string& label, TPPixel colour);

  //##################################################################################################
  void rangePop();

  //################################################################################################
  void viewResults(const std::function<void(const ProfilerResults&)>& closure);

  //################################################################################################
  void addSummaryGenerator(const std::string& label, const std::function<double(const std::vector<ProgressEvent>&)>& calcFunc, const std::function<std::string(double)>& printer = nullptr);

protected:
  //##################################################################################################
  void reset();

  //##################################################################################################
  bool record();

  //##################################################################################################
  bool stop();

private:
  friend class ProfilerController;
  struct Private;
  friend struct Private;
  Private* d;
};

//##################################################################################################
class ScopedProfilerRange
{
//##################################################################################################
  ScopedProfilerRange() = delete;
public:
//##################################################################################################
  ScopedProfilerRange(Profiler* profiler_, const std::string label_, TPPixel colour_):
  profiler(profiler_)
  {
    if(profiler)
      profiler->rangePush(label_, colour_);
  }

//##################################################################################################
  ~ScopedProfilerRange()
  {
    if(profiler)
      profiler->rangePop();
  }

private:
  Profiler* profiler;
};

}

#ifdef TP_ENABLE_PROFILING
#define INITIALIZE_PROFILER(name, profiler) profiler = std::make_unique<tp_utils::Profiler>(name);
#else
#define INITIALIZE_PROFILER(name, profiler) profiler.reset()
#endif

#ifdef TP_ENABLE_PROFILING
#define PRF_ADD_SUMMARY_GENERATOR(profiler, label, calcFunc, printer) if(profiler) profiler->addSummaryGenerator(label, calcFunc, printer)
#else
#define PRF_ADD_SUMMARY_GENERATOR(profiler, label, calcFunc, printer)
#endif

#ifdef TP_ENABLE_PROFILING
#define PRF_RANGE_PUSH(profiler, label, ...) if(profiler) profiler->rangePush(label, __VA_ARGS__)
#else
#define PRF_RANGE_PUSH(profiler, label, ...)
#endif

#ifdef TP_ENABLE_PROFILING
#define PRF_RANGE_POP(profiler) if(profiler) profiler->rangePop()
#else
#define PRF_RANGE_POP(profiler)
#endif

#ifdef TP_ENABLE_PROFILING
#define PRF_SCOPED_RANGE(profiler, label, ...) tp_utils::ScopedProfilerRange scopedPrfRange##__FILE__##__LINE__(profiler, label, __VA_ARGS__)
#else
#define PRF_SCOPED_RANGE(profiler, label, ...)
#endif

#endif //tp_utils_Profiler_h
