#include "tp_utils/detail/log_stats/impl.h"
#include "tp_utils/TimeUtils.h"

namespace tp_utils
{

namespace detail
{

//##################################################################################################
struct VirtualMemory
{
  size_t VmSize{0};
  size_t VmHWM{0};
  size_t VmRSS{0};

  //################################################################################################
  VirtualMemory()
  {
#ifdef TP_LINUX
  //https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
  auto parseLine = [](char* line)
  {
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
  };

  //Note: this value is in KB!
  auto getValue = [=](const std::string& key)
  {
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
      if (strncmp(line, key.c_str(), key.size()) == 0){
        result = parseLine(line);
        break;
      }
    }
    fclose(file);
    return result;
  };

  VmSize = size_t(getValue("VmSize:"));
  VmHWM = size_t(getValue("VmHWM:"));
  VmRSS = size_t(getValue("VmRSS:"));
#endif
  }
};

}

//##################################################################################################
inline void addMemoryUsageProducer(detail::KeyValueLogStatsTimer& keyValueStatsTimer)
{
#ifndef TP_LINUX
  TP_UNUSED(keyValueStatsTimer);
#else
  keyValueStatsTimer.addProducer("Memory ", [=]
  {
    detail::VirtualMemory virtualMemory;

    std::map<std::string, size_t> result;
    result["VmSize"] = virtualMemory.VmSize;
    result["VmHWM"]  = virtualMemory.VmHWM;
    result["VmRSS"]  = virtualMemory.VmRSS;
    return result;
  });
#endif
}

}
