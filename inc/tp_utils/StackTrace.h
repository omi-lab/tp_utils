#ifndef tp_utils_StackTrace_h
#define tp_utils_StackTrace_h

#include "tp_utils/Globals.h"

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
void TP_UTILS_SHARED_EXPORT printStackTrace();

//##################################################################################################
//! Returns a stack trace as a string
std::string TP_UTILS_SHARED_EXPORT formatStackTrace();

}

#endif
