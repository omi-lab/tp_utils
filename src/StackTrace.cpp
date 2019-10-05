#include "tp_utils/StackTrace.h"
#include "tp_utils/DebugUtils.h"

//PLATFORM_ABSTRACTIONS
#if defined(tp_qt_ANDROID)
#  define ANDROID_STACKTRACE
#elif defined(tp_qt_OSX)
#  define OSX_STACKTRACE
#elif defined(EMSCRIPTEN)
#  define EMSCRIPTEN_STACKTRACE
#elif defined(__GLIBC__) && (!defined(__UCLIBC__) || defined(__UCLIBC_HAS_BACKTRACE__))
#  define GCC_STACKTRACE
#endif

#ifdef GCC_STACKTRACE
//This allows us to print a stack trace
//This is gcc specific, and we may want to remove it from production code
#include <execinfo.h>
#include <cxxabi.h>
#include <ucontext.h>
#include <cstring>
#include <memory>
#endif

#if defined(ANDROID_STACKTRACE)
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>
#endif

#if defined(EMSCRIPTEN_STACKTRACE)
#include <emscripten.h>
#endif

#define MAX_LEVELS 100

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

#if defined(GCC_STACKTRACE)
namespace
{
const int MAX_TRACE_SIZE = 384;
}
#endif

namespace tp_utils
{
//##################################################################################################
//-- GCC -------------------------------------------------------------------------------------------
//##################################################################################################
#if defined(GCC_STACKTRACE)
static bool demangle(const char* symbol, std::string& output)
{
  std::unique_ptr<char, decltype(&free)> symbol2(strdup(symbol), &free);

  // find parentheses and +address offset surrounding the mangled name:
  // ./module(function+0x15c) [0x8048a6d]
  char* begin_name = nullptr;
  char* end_name = nullptr;
  char* begin_offset = nullptr;
  char* end_offset = nullptr;
  char* p;
  for(p = symbol2.get(); *p; ++p)
  {
    if(*p == '(')
      begin_name = p;
    else if(*p == '+')
      begin_offset = p;
    else if((*p == ')') && begin_offset)
    {
      end_offset = p;
      break;
    }
  }

  // BCH 24 Dec 2014: backtrace_symbols() on OS X seems to return strings in a different, non-standard format.
  // Added this code in an attempt to parse that format.  No doubt it could be done more cleanly.  :->
  // Updated to correctly parse the symbols on iOS too.
  if(!begin_name)
  {
    begin_offset = nullptr;
    end_offset = nullptr;

    enum class ParseState
    {
      kInWhitespace1 = 1,
      kInLineNumber,
      kInWhitespace2,
      kInPackageName,
      kInWhitespace3,
      kInAddress,
      kInWhitespace4,
      kInFunction,
      kInWhitespace5,
      kInPlus,
      kInWhitespace6,
      kInOffset,
      kInOverrun
    };

    ParseState parse_state = ParseState::kInWhitespace1;
    for(p = symbol2.get(); *p; ++p)
    {
      switch(parse_state)
      {
        case ParseState::kInWhitespace1: if(!isspace(*p)) parse_state = ParseState::kInLineNumber;  break;
        case ParseState::kInLineNumber:  if(isspace(*p))  parse_state = ParseState::kInWhitespace2; break;
        case ParseState::kInWhitespace2: if(!isspace(*p)) parse_state = ParseState::kInPackageName; break;
        case ParseState::kInPackageName: if(isspace(*p))  parse_state = ParseState::kInWhitespace3; break;
        case ParseState::kInWhitespace3: if(!isspace(*p)) parse_state = ParseState::kInAddress;     break;
        case ParseState::kInAddress:     if(isspace(*p))  parse_state = ParseState::kInWhitespace4; break;
        case ParseState::kInWhitespace4:
          if(!isspace(*p))
          {
            parse_state = ParseState::kInFunction;
            begin_name = p - 1;
          }
          break;

        case ParseState::kInFunction:
          if(isspace(*p))
          {
            parse_state = ParseState::kInWhitespace5;
            end_name = p;
          }
          break;

        case ParseState::kInWhitespace5:
          if(!isspace(*p))
          {
            if(*p == '+')
              parse_state = ParseState::kInPlus;
            else
            {
              parse_state = ParseState::kInOffset;
              begin_offset = p - 1;
            }
          }
          break;

        case ParseState::kInPlus:        if(isspace(*p)) parse_state = ParseState::kInWhitespace6;  break;
        case ParseState::kInWhitespace6:
          if(!isspace(*p))
          {
            parse_state = ParseState::kInOffset;
            begin_offset = p - 1;
          }
          break;

        case ParseState::kInOffset:
          if(isspace(*p))
          {
            parse_state = ParseState::kInOverrun;
            end_offset = p;
          }
          break;

        case ParseState::kInOverrun:
          break;
      }
    }

    if((parse_state == ParseState::kInOffset) && !end_offset)
      end_offset = p;
  }

  if(begin_name && begin_offset && end_offset && (begin_name < begin_offset))
  {
    *begin_name++ = '\0';
    if(end_name)
      *end_name = '\0';

    *begin_offset++ = '\0';
    *end_offset = '\0';

    // mangled name is now in [begin_name, begin_offset) and caller
    // offset in [begin_offset, end_offset). now apply __cxa_demangle():

    int status = -1;
    std::unique_ptr<char, decltype(&free)> demangled(abi::__cxa_demangle(begin_name, nullptr, nullptr, &status), &free);
    if(demangled && !status)
    {
      output += demangled.get();
      output += " + ";
      output += begin_offset;
    }
    else
    {
      // demangling failed. Output function name as a C function with no arguments.
      output += begin_name;
      output += "() + ";
      output += begin_offset;
    }
  }
  else
  {
    // couldn't parse the line? Just print the whole line.
    output += symbol;
    output[MAX_TRACE_SIZE-1] = '\0';
  }

  return true;
}

//##################################################################################################
void TP_UTILS_SHARED_EXPORT printStackTrace()
{
  //Get the backtrace
  std::array<void*, MAX_LEVELS> array = tpMakeArray<void*, MAX_LEVELS>(nullptr);

  int startOffset = 1;   // don't include printStackTrace() in the output
#if defined(__mips)
  int size = backtraceMIPS(array, MAX_LEVELS);
  ucontext_t* context = (ucontext_t*)pcontext;
  if(context)
  {
    // When called from the CrashReporter signal handler, the first two entries on the stack are the
    // signal handler and the sigaction() address where the signal handler was called from. Insert
    // the address of the last caller before the signal was generated.
    array[0] = array[1];
    array[1] = array[2];
    array[2] = (void*)context->uc_mcontext.pc;
    startOffset = 0;
  }
#else
  int size = backtrace(array.data(), MAX_LEVELS);
#endif
  std::unique_ptr<char*, decltype(&free)> strings(backtrace_symbols(array.data(), size), &free);
  tpWarning() << "Stack frames: " << size - startOffset;
  for(int i = startOffset; i < size; ++i)
  {
    const char* symbol = strings.get()[i];

    //Extract name and address
    std::string demangled;
    if(demangle(symbol, demangled))
      tpWarning() << "Frame " << i << ": " << demangled;
    else
      tpWarning() << "Frame " << i << ": " << symbol;
  }
}

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT formatStackTrace()
{
  //Get the backtrace
  std::array<void*, MAX_LEVELS> array = tpMakeArray<void*, MAX_LEVELS>(nullptr);

  int startOffset = 1;   // don't include printStackTrace() in the output
  int size = backtrace(array.data(), MAX_LEVELS);

  //Convert the backtrace to strings
  std::unique_ptr<char*, decltype(&free)> strings(backtrace_symbols(array.data(), size), &free);

  std::string results = std::string("Stack frames: ") + std::to_string(size - startOffset) + '\n';
  for(int i = startOffset; i < size; ++i)
    results.append(std::string("Frame ") + std::to_string(i) +": " + strings.get()[i]) + '\n';

  return results;
}

#elif defined(EMSCRIPTEN_STACKTRACE)
//##################################################################################################
void TP_UTILS_SHARED_EXPORT printStackTrace()
{
  emscripten_run_script("console.log(stackTrace());");
}

#elif defined(ANDROID_STACKTRACE)
// Android version taken from:
// http://stackoverflow.com/questions/8115192/android-ndk-getting-the-backtrace/35585744#35585744
struct android_backtrace_state
{
  void** current;
  void** end;
};

//##################################################################################################
static _Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context, void* arg)
{
  android_backtrace_state* state = (android_backtrace_state*)arg;
  uintptr_t pc = _Unwind_GetIP(context);
  if(pc)
  {
    if(state->current == state->end)
      return _URC_END_OF_STACK;
    else
      *state->current++ = static_cast<void*>(pc);
  }

  return _URC_NO_REASON;
}

//##################################################################################################
void TP_UTILS_SHARED_EXPORT printStackTrace()
{
  tpWarning() << "\nandroid stack dump";

  const int max = 100;
  void* buffer[max];

  android_backtrace_state state;
  state.current = buffer;
  state.end = buffer + max;

  _Unwind_Backtrace(android_unwind_callback, &state);

  char traceString[512];
  int count = (int)(state.current - buffer);
  for(int idx = 0; idx < count; idx++)
  {
    const void* addr = buffer[idx];
    const char* symbol = "";

    Dl_info info;
    if(dladdr(addr, &info) && info.dli_sname)
      symbol = info.dli_sname;

    int status = -1;
    char* demangled = __cxxabiv1::__cxa_demangle(symbol, 0, 0, &status);

    sprintf(traceString, "%03d: 0x%p %s", idx, addr, (demangled && !status) ? demangled : symbol);
    tpWarning() << traceString;

    if(!status)
      free(demangled);
  }

  tpWarning() << "android stack dump done\n";
}

std::string TP_UTILS_SHARED_EXPORT formatStackTrace()
{
  return std::string();
}

//##################################################################################################
//-- Not Supported ---------------------------------------------------------------------------------
//##################################################################################################
#else
void TP_UTILS_SHARED_EXPORT printStackTrace()
{
}

std::string TP_UTILS_SHARED_EXPORT formatStackTrace()
{
  return std::string();
}
#endif

}
