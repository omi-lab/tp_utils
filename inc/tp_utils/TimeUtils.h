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
int64_t TP_UTILS_SHARED_EXPORT currentTime();

//##################################################################################################
//! Returns the current time as ms since SinceEpoch
/*!
\return the current time as ms since since the epoch
*/
int64_t TP_UTILS_SHARED_EXPORT currentTimeMS();

//##################################################################################################
class TP_UTILS_SHARED_EXPORT ElapsedTimer
{
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
class TP_UTILS_SHARED_EXPORT FunctionTimeStats
{
public:
  //################################################################################################
  static void add(int64_t timeMicroseconds, const char* file, int line);

  //################################################################################################
  static std::string takeResults();

  //################################################################################################
  static std::map<std::string, size_t> keyValueResults();

private:
  struct Private;
  static Private* instance();
};

class FunctionTimer
{
public:
  FunctionTimer(const char* file, int line);
  ~FunctionTimer();
private:
  int64_t m_start;
  const char* m_file;
  int m_line;
};

#define TP_FUNCTION_TIME tp_utils::FunctionTimer TP_CONCAT(tpFunctionTimer, __LINE__)(__FILE__, __LINE__); TP_UNUSED(TP_CONCAT(tpFunctionTimer, __LINE__))
#else
#define TP_FUNCTION_TIME do{}while(false)
#endif

}

#endif
