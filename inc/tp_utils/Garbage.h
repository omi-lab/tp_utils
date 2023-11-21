#ifndef tp_utils_Garbage_h
#define tp_utils_Garbage_h

#include "tp_utils/Globals.h" // IWYU pragma: keep

#include <functional>

namespace tp_utils
{

//##################################################################################################
void initGarbage();

//##################################################################################################
void garbage(const std::function<void()>& closure);

}

#endif
