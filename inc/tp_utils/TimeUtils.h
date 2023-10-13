#ifndef tp_utils_TimeUtils_h
#define tp_utils_TimeUtils_h

#include "tp_utils/Globals.h"

#include <map>

namespace tp_utils
{

//##################################################################################################
//! Returns the current time as a 64 bit Unix time stamp
/*!
\return the current time as a 64 bit Unix time stamp
*/
int64_t TP_UTILS_EXPORT currentTime();

//##################################################################################################
//! Returns the current time as ms since SinceEpoch
/*!
\return the current time as ms since since the epoch
*/
int64_t TP_UTILS_EXPORT currentTimeMS();

//##################################################################################################
//! Returns the current time as us since SinceEpoch
/*!
\return the current time as us since since the epoch
*/
int64_t TP_UTILS_EXPORT currentTimeMicroseconds();
    
//##################################################################################################
class TP_UTILS_EXPORT ElapsedTimer
{
  TP_NONCOPYABLE(ElapsedTimer);
public:
  //################################################################################################
  ElapsedTimer(int64_t smallTime=-1);

  //################################################################################################
  ~ElapsedTimer();

  //################################################################################################
  void start();

  //################################################################################################
  int64_t restart();

  //################################################################################################
  int64_t elapsed() const;

  //################################################################################################
  //! Print the current time and restart.
  void printTime(const char* msg);

private:
  struct Private;
  Private* d;
};

#ifdef TP_ENABLE_TIME_SCOPE
#define TP_TIME_SCOPE(name) tp_utils::ElapsedTimer TP_CONCAT(tpET, __LINE__)(20); TP_CONCAT(tpET, __LINE__).start(); TP_CLEANUP([&]{TP_CONCAT(tpET, __LINE__).printTime(name);})
#else
#define TP_TIME_SCOPE(name)[]{}()
#endif

#ifdef TP_ENABLE_FUNCTION_TIME

//##################################################################################################
struct FunctionTimeReading
{
  int64_t start{0};
  int64_t timeTaken{0};
  std::string key;
};

//##################################################################################################
class TP_UTILS_EXPORT FunctionTimeStats
{
public:
  //################################################################################################
  static void add(const std::vector<FunctionTimeReading>& readings);

  //################################################################################################
  static std::string takeResults();

  //################################################################################################
  static void reset();

  //################################################################################################
  static bool isMainThread();

  //################################################################################################
  static std::map<std::string, size_t> keyValueResults();

private:
  struct Private;
  static Private* instance();
};

//################################################################################################
class FunctionTimer
{
public:
  //################################################################################################
  FunctionTimer(const char* file, int line, const char* name);

  //################################################################################################
  ~FunctionTimer();

  //################################################################################################
  void finishStep(const char* name);

private:
  size_t m_index{0};
#ifdef TP_ENABLE_TIME_SCOPE
  size_t m_stepIndex{0};
#endif
};

#define TP_FUNCTION_TIME(A) tp_utils::FunctionTimer TP_CONCAT(tpFunctionTimer, __LINE__)(__FILE__, __LINE__, A); TP_UNUSED(TP_CONCAT(tpFunctionTimer, __LINE__))
#else

//################################################################################################
class FunctionTimer
{
public:
  //################################################################################################
  FunctionTimer(const char* file, int line, const char* name)
  {
    TP_UNUSED(file);
    TP_UNUSED(line);
    TP_UNUSED(name);
  }

  //################################################################################################
  ~FunctionTimer()
  {

  }

  //################################################################################################
  int64_t ellapsed() const
  {
    return 0;
  }

  //################################################################################################
  void finishStep(const char* name)
  {
    TP_UNUSED(name);
  }
};

#define TP_FUNCTION_TIME(A) do{}while(false)
#endif

}

#endif
