#include "tp_utils/StackTrace.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/FileUtils.h"
//PLATFORM_ABSTRACTIONS
#if defined(TP_ANDROID)
#  define ANDROID_STACKTRACE
#elif defined(TP_OSX)
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

#if defined(OSX_STACKTRACE)
#include <execinfo.h>
#include <stdio.h>
#endif

#if defined(TP_WIN32_MSVC)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <new.h>
#include <signal.h>
#include <exception>
#include <sys/stat.h>
#include <Psapi.h>
#include <rtcapi.h>
#include <shellapi.h>
//#include <DbgHelp.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")

// Some versions of imagehlp.dll lack the proper packing directives themselves
// so we need to do it.
#pragma pack( push, before_imagehlp, 8 )
#include <ImageHlp.h>
#pragma pack( pop, before_imagehlp )

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
  }

  return true;
}

//##################################################################################################
void TP_UTILS_EXPORT printStackTrace()
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
void TP_UTILS_EXPORT printAddr2Line()
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
  std::string output;
  for(int i = startOffset; i < size; ++i)
  {
    const char* symbol = strings.get()[i];

    std::vector<std::string> partsA;
    tpSplit(partsA, symbol, '(', SplitBehavior::SkipEmptyParts);
    if(partsA.size()==2)
    {
      std::vector<std::string> partsB;
      tpSplit(partsB, partsA.back(), '[', SplitBehavior::SkipEmptyParts);
      if(!partsB.empty())
      {
        std::vector<std::string> partsC;
        tpSplit(partsC, partsB.back(), ']', SplitBehavior::SkipEmptyParts);

        if(!partsC.empty())
        {
          output += " addr2line ";
          output += partsC.front();
          output += " -f -C -e ";
          output += partsA.front();
          output += '\n';
        }
      }
    }
  }

  std::cout << output << std::endl;
}

//##################################################################################################
std::string TP_UTILS_EXPORT formatStackTrace()
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
void TP_UTILS_EXPORT printStackTrace()
{
  emscripten_run_script("console.log((new Error()).stack);");
}

//##################################################################################################
std::string TP_UTILS_EXPORT formatStackTrace()
{
  return emscripten_run_script_string("(new Error()).stack;");
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
      *state->current++ = reinterpret_cast<void*>(pc);
  }

  return _URC_NO_REASON;
}

//##################################################################################################
void TP_UTILS_EXPORT printStackTrace()
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

std::string TP_UTILS_EXPORT formatStackTrace()
{
  return std::string();
}

#elif defined(OSX_STACKTRACE)

//##################################################################################################
void TP_UTILS_EXPORT printStackTrace()
{
  void* callstack[128];
  int i, frames = backtrace(callstack, 128);
  char** strs = backtrace_symbols(callstack, frames);
  for(i = 0; i < frames; ++i)
    tpWarning() << strs[i];
  free(strs);
}

//##################################################################################################
std::string TP_UTILS_EXPORT formatStackTrace()
{
  return std::string();
}

#elif defined(TP_WIN32_MSVC)

//##################################################################################################
// This method creates minidump of the process
void createMiniDump(EXCEPTION_POINTERS* pExceptionPtrs)
{
  if(!pExceptionPtrs || !pExceptionPtrs->ContextRecord)
    return;

  HMODULE hDbgHelp = nullptr;
  HANDLE hFile = nullptr;
  MINIDUMP_EXCEPTION_INFORMATION mei;
  MINIDUMP_CALLBACK_INFORMATION mci;

  // Load dbghelp.dll
  hDbgHelp = LoadLibrary(_T("dbghelp.dll"));
  if(hDbgHelp==nullptr)
  {
    // Error - couldn't load dbghelp.dll
    return;
  }

  // Create the minidump file
  hFile = CreateFile(
        _T("crashdump.dmp"),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

  if(hFile==INVALID_HANDLE_VALUE)
  {
    // Couldn't create file
    return;
  }

  // Write minidump to the file
  mei.ThreadId = GetCurrentThreadId();
  mei.ExceptionPointers = pExceptionPtrs;
  mei.ClientPointers = FALSE;
  mci.CallbackRoutine = nullptr;
  mci.CallbackParam = nullptr;

  typedef BOOL (WINAPI *LPMINIDUMPWRITEDUMP)(
        HANDLE hProcess,
        DWORD ProcessId,
        HANDLE hFile,
        MINIDUMP_TYPE DumpType,
        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
        CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam,
        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

  LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump = reinterpret_cast<LPMINIDUMPWRITEDUMP>(GetProcAddress(hDbgHelp, "MiniDumpWriteDump"));
  if(!pfnMiniDumpWriteDump)
  {
    // Bad MiniDumpWriteDump function
    return;
  }

  HANDLE hProcess = GetCurrentProcess();
  DWORD dwProcessId = GetCurrentProcessId();

  BOOL bWriteDump = pfnMiniDumpWriteDump(
        hProcess,
        dwProcessId,
        hFile,
        MiniDumpNormal,
        &mei,
        nullptr,
        &mci);

  if(!bWriteDump)
  {
    // Error writing dump.
    return;
  }

  // Close file
  CloseHandle(hFile);

  // Unload dbghelp.dll
  FreeLibrary(hDbgHelp);
}

//##################################################################################################
// The following code gets exception pointers using a workaround found in CRT code.
void getExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS** ppExceptionPointers)
{
  // The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

  EXCEPTION_RECORD ExceptionRecord;
  CONTEXT ContextRecord;
  memset(&ContextRecord, 0, sizeof(CONTEXT));
  ContextRecord.ContextFlags = CONTEXT_FULL;

#ifdef _X86_

  __asm {
    mov dword ptr [ContextRecord.Eax], eax
        mov dword ptr [ContextRecord.Ecx], ecx
        mov dword ptr [ContextRecord.Edx], edx
        mov dword ptr [ContextRecord.Ebx], ebx
        mov dword ptr [ContextRecord.Esi], esi
        mov dword ptr [ContextRecord.Edi], edi
        mov word ptr [ContextRecord.SegSs], ss
        mov word ptr [ContextRecord.SegCs], cs
        mov word ptr [ContextRecord.SegDs], ds
        mov word ptr [ContextRecord.SegEs], es
        mov word ptr [ContextRecord.SegFs], fs
        mov word ptr [ContextRecord.SegGs], gs
        pushfd
        pop [ContextRecord.EFlags]
  }

  ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
  ContextRecord.Eip = (ULONG)_ReturnAddress();
  ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
  ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress()-1);


#elif defined (_IA64_) || defined (_AMD64_)

  /* Need to fill up the Context in IA64 and AMD64. */
  RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) || defined (_AMD64_) */

  ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) || defined (_AMD64_) */

  ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));

  ExceptionRecord.ExceptionCode = dwExceptionCode;
  ExceptionRecord.ExceptionAddress = _ReturnAddress();

  ///

  EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
  memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
  CONTEXT* pContextRecord = new CONTEXT;
  memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));

  *ppExceptionPointers = new EXCEPTION_POINTERS;
  (*ppExceptionPointers)->ExceptionRecord = pExceptionRecord;
  (*ppExceptionPointers)->ContextRecord = pContextRecord;
}

//##################################################################################################
[[noreturn]]void saveCrashReportAndExit()
{
  saveCrashReportAndExit(nullptr);
}

//##################################################################################################
[[noreturn]]void saveCrashReportAndExit(EXCEPTION_POINTERS* pExceptionPtrs)
{
  if(!pExceptionPtrs)
    getExceptionPointers(0, &pExceptionPtrs);

  printStackTrace(pExceptionPtrs);
  tp_utils::writeTextFile("crash.txt", formatStackTrace(pExceptionPtrs));
  createMiniDump(pExceptionPtrs);
  exit(1);
}

namespace
{

//##################################################################################################
struct module_data
{
  std::string image_name;
  std::string module_name;
  void *base_address;
  DWORD load_size;
};

//##################################################################################################
class symbol
{
  typedef IMAGEHLP_SYMBOL64 sym_type;
  sym_type *sym;
  static const int max_name_len = 1024;
public:
  //################################################################################################
  symbol(HANDLE process, DWORD64 address):
    sym(reinterpret_cast<sym_type*>(::operator new(sizeof(*sym) + max_name_len)))
  {
    memset(sym, '\0', sizeof(*sym) + max_name_len);
    sym->SizeOfStruct = sizeof(*sym);
    sym->MaxNameLength = max_name_len;
    DWORD64 displacement;

    if(!SymGetSymFromAddr64(process, address, &displacement, sym))
      std::cout << "Bad symbol." << std::endl;
  }

  //################################################################################################
  std::string undecorated_name()
  {
    std::vector<char> und_name(max_name_len);
    UnDecorateSymbolName(sym->Name, &und_name[0], max_name_len, UNDNAME_COMPLETE);
    return std::string(&und_name[0], strlen(&und_name[0]));
  }
};
}

//##################################################################################################
std::string formatStackTrace(EXCEPTION_POINTERS* pExceptionPtrs)
{
  std::string result;
  if(!pExceptionPtrs || !pExceptionPtrs->ContextRecord)
  {
    result += "No stack trace available.";
    return result;
  }

  HANDLE process = GetCurrentProcess();
  SymInitialize(process, nullptr, TRUE);

  // StackWalk64() may modify context record passed to it, so we will
  // use a copy.
  CONTEXT context_record = *pExceptionPtrs->ContextRecord;
  // Initialize stack walking.
  STACKFRAME64 stackFrame;
  memset(&stackFrame, 0, sizeof(stackFrame));
#if defined(_WIN64)
  int machine_type = IMAGE_FILE_MACHINE_AMD64;
  stackFrame.AddrPC.Offset = context_record.Rip;
  stackFrame.AddrFrame.Offset = context_record.Rbp;
  stackFrame.AddrStack.Offset = context_record.Rsp;
#else
  int machine_type = IMAGE_FILE_MACHINE_I386;
  stackFrame.AddrPC.Offset = context_record.Eip;
  stackFrame.AddrFrame.Offset = context_record.Ebp;
  stackFrame.AddrStack.Offset = context_record.Esp;
#endif
  stackFrame.AddrPC.Mode = AddrModeFlat;
  stackFrame.AddrFrame.Mode = AddrModeFlat;
  stackFrame.AddrStack.Mode = AddrModeFlat;

  int frameNumber=0;
  DWORD offsetFromSymbol=0;
  while(StackWalk64(DWORD(machine_type),
                    GetCurrentProcess(),
                    GetCurrentThread(),
                    &stackFrame,
                    &context_record,
                    nullptr,
                    &SymFunctionTableAccess64,
                    &SymGetModuleBase64,
                    nullptr))
  {
    result += tp_utils::fixedWidthKeepRight(std::to_string(frameNumber), 3, '0') + ' ';

    if(stackFrame.AddrPC.Offset!=0)
    {
      result += symbol(process, stackFrame.AddrPC.Offset).undecorated_name();

      IMAGEHLP_LINE64 line;
      line.SizeOfStruct = sizeof(line);
      if(SymGetLineFromAddr64(process, stackFrame.AddrPC.Offset, &offsetFromSymbol, &line))
        result += ' ' + std::string(line.FileName) + ':' + std::to_string(line.LineNumber);

      result += '\n';
    }
    else
      result += "(No Symbols: PC == 0)\n";
    frameNumber++;
  }
  return result;
}

//##################################################################################################
std::string formatStackTrace()
{
  EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
  getExceptionPointers(0, &pExceptionPtrs);
  return formatStackTrace(pExceptionPtrs);
}

//##################################################################################################
void printStackTrace()
{
  std::cout << formatStackTrace() << std::endl;
}

//##################################################################################################
void printStackTrace(EXCEPTION_POINTERS* pExceptionPtrs)
{
  std::cout << formatStackTrace(pExceptionPtrs) << std::endl;
}

//##################################################################################################
//-- Not Supported ---------------------------------------------------------------------------------
//##################################################################################################
#else
void TP_UTILS_EXPORT printStackTrace()
{
}

std::string TP_UTILS_EXPORT formatStackTrace()
{
  return std::string();
}
#endif


//##################################################################################################
void saveCrashReport()
{
  printStackTrace();
  tp_utils::writeTextFile("crash.txt", formatStackTrace());
}

//##################################################################################################
#ifndef TP_WIN32_MSVC
[[noreturn]]void saveCrashReportAndExit()
{
  saveCrashReport();
  exit(1);
}
#endif
}
