#ifdef TP_ENABLE_TRANSLATION
#include "tp_utils/Translation.h"
#include "tp_utils/MutexUtils.h"


namespace tp_utils
{

//##################################################################################################
void translationTable(const std::function<void(std::map<std::string, const char*>&)>& closure)
{
  static TPMutex mutex{TPM};
  static std::map<std::string, const char*> table;
  TP_MUTEX_LOCKER(mutex);
  closure(table);
}

//##################################################################################################
const char* translate(const char* str, const char* file, int line)
{
  std::string key = std::string(str) + std::string(file) + std::to_string(line);
  translationTable([&](auto& table)
  {
    if(auto i=table.find(key); i!=table.end())
      str = i->second;
  });
  return str;
}

}
#else

int translationTable{0};

#endif
