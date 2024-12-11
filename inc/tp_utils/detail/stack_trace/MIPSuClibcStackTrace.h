//##################################################################################################
#if defined(__mips)
// Currently, uClibc backtrace() doesn't work if it's called in a shared library.
// This calls the actual unwind code, _Unwind_Backtrace() in libgcc_s.so, directly.
#include <unwind.h>
#include <dlfcn.h>
extern "C" {
extern _Unwind_Reason_Code backtrace_helper(struct _Unwind_Context* ctx, void* a);
}

namespace
{
struct TraceArg
{
  void** array;
  int    count;
  int    size;
};

int backtraceMIPS(void** array, int size)
{
  if(size < 1)
    return 0;

  typedef _Unwind_Reason_Code (*UwBacktrace)(_Unwind_Trace_Fn, void*);
  static _Unwind_Reason_Code (*unwindBacktrace)(_Unwind_Trace_Fn, void*) = nullptr;
  if(!unwindBacktrace)
  {
    void* libHandle = dlopen("libgcc_s.so.1", RTLD_LAZY);
    if(!libHandle)
    {
      tpWarning() << "StackTrace: unable to open libgcc_s.so.1";
      return 0;
    }
    unwindBacktrace = (UwBacktrace)dlsym(libHandle, "_Unwind_Backtrace");
    if(!unwindBacktrace)
    {
      tpWarning() << "StackTrace: unable to access _Unwind_Backtrace in libgcc.so.1";
      return 0;
    }
  }

  TraceArg traceArg;
  traceArg.array = array;
  traceArg.count = -1;
  traceArg.size  = size;
  unwindBacktrace(backtrace_helper, &traceArg);
  return traceArg.count != -1 ? traceArg.count : 0;
}
}
#endif
