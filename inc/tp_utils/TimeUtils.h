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
class TP_UTILS_EXPORT FunctionTimeStats
{
public:
  //################################################################################################
  static void add(int64_t timeMicroseconds, const char* file, int line, const std::string& name);

  //################################################################################################
  static std::string takeResults();

  //################################################################################################
  static void reset();

  //################################################################################################
  static std::map<std::string, size_t> keyValueResults();

private:
  struct Private;
  static Private* instance();
};

class FunctionTimer
{
public:
  FunctionTimer(const char* file, int line, const std::string& name);
  ~FunctionTimer();
private:
  int64_t m_start;
  const char* m_file;
  int m_line;
  std::string m_name;
};

#define TP_FUNCTION_TIME(A) tp_utils::FunctionTimer TP_CONCAT(tpFunctionTimer, __LINE__)(__FILE__, __LINE__, A); TP_UNUSED(TP_CONCAT(tpFunctionTimer, __LINE__))
#else
#define TP_FUNCTION_TIME(A) do{}while(false)
#endif

}

#endif
