#ifndef tp_utils_Profiler_h
#define tp_utils_Profiler_h

#ifdef TP_ENABLE_PROFILING

#include "tp_utils/TPPixel.h"
#include "tp_utils/StringID.h"

#include "json.hpp"

#include <string>


namespace tp_utils
{
class Profiler;
class ProfilerController;
struct ProgressEvent;

//##################################################################################################
using SummaryGenerator = std::function<void(const Profiler&, std::vector<std::pair<std::string, std::string>>&)>;

//##################################################################################################
class Profiler
{ 
  TP_NONCOPYABLE(Profiler);

  //################################################################################################
  Profiler(ProfilerController* controller, const StringID& id_);
public:

  //################################################################################################
  ~Profiler();

  //################################################################################################
  const StringID id;

  //################################################################################################
  std::string name() const;

  //################################################################################################
 void setName(const std::string& name);

  //################################################################################################
  void rangePush(const std::string& label, TPPixel colour);

  //################################################################################################
  void rangePop();

  //################################################################################################
  void viewProgressEvents(const std::function<void(const std::vector<ProgressEvent>&)>& closure) const;

  //################################################################################################
  std::vector<std::pair<std::string, std::string>> summaries() const;

  //################################################################################################
  void clearSummaryGenerators();

  //################################################################################################
  void addSummaryGenerator(const SummaryGenerator& summaryGenerator);

  //################################################################################################
  void startRecording();

  //################################################################################################
  void stopRecording();

  //################################################################################################
  bool isRecording() const;

  //################################################################################################
  nlohmann::json saveState() const;

private:
  friend class ProfilerController;
  struct Private;
  friend struct Private;
  Private* d;
};

//##################################################################################################
class ScopedProfilerRange
{
  TP_NONCOPYABLE(ScopedProfilerRange);
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
