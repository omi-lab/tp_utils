#ifndef tp_utils_Translation_h
#define tp_utils_Translation_h

#include "tp_utils/Globals.h"

#include <map>

#ifdef TP_ENABLE_TRANSLATION

namespace tp_utils
{

//##################################################################################################
void translationTable(const std::function<void(std::map<std::string, const char*>&)>& closure);

//##################################################################################################
const char* translate(const char* str, const char* file, int line);

}

#define tpTR(str)tp_utils::translate(str,__FILE__,__LINE__)
#else
#define tpTR(str)str
#endif

#endif
