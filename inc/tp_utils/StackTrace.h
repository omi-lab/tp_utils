#ifndef tp_utils_StackTrace_h
#define tp_utils_StackTrace_h

#include "tp_utils/Globals.h"

#ifdef TP_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace tp_utils
{

//##################################################################################################
//! Prints a stack trace to tpWarning
/*!
If called from a signal handler, the \p context parameter should be supplied. In this case, the
first two entries on the call stack are ignored, and the first entry is obtained from the supplied
context.

Otherwise, all entries on the stack are printed.

\param context - Pointer to a ucontext_t structure supplied to signal handler, or null if not
                 called by signal handler.
*/
void TP_UTILS_EXPORT printStackTrace();

//##################################################################################################
std::vector<std::string> TP_UTILS_EXPORT addr2Line();

//##################################################################################################
void TP_UTILS_EXPORT printAddr2Line();

//##################################################################################################
void TP_UTILS_EXPORT execAddr2Line();

//##################################################################################################
//! Returns a stack trace as a string
std::string TP_UTILS_EXPORT formatStackTrace();

//##################################################################################################
void saveCrashReport();

//##################################################################################################
[[noreturn]]void saveCrashReportAndExit();

#ifdef TP_WIN32_MSVC
//##################################################################################################
void TP_UTILS_EXPORT createMiniDump(EXCEPTION_POINTERS* pExceptionPtrs);

//##################################################################################################
[[noreturn]]void TP_UTILS_EXPORT saveCrashReportAndExit(EXCEPTION_POINTERS* pExceptionPtrs);

//##################################################################################################
void TP_UTILS_EXPORT printStackTrace(EXCEPTION_POINTERS* pExceptionPtrs);

//##################################################################################################
std::string TP_UTILS_EXPORT formatStackTrace(EXCEPTION_POINTERS* pExceptionPtrs);

#endif

}

#endif
