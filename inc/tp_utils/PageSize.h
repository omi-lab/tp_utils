#ifndef tp_utils_PageSize_h
#define tp_utils_PageSize_h

#include "tp_utils/Globals.h"

namespace tp_utils
{

//##################################################################################################
//! Returns the virtual memory page size or 0
size_t TP_UTILS_SHARED_EXPORT pageSize();

}

#ifdef TP_LINUX //=================================================================================

#include <sys/mman.h>
namespace tp_utils
{
//##################################################################################################
inline size_t TP_UTILS_SHARED_EXPORT pageSize()
{
  size_t n;
  int u;
  for(n = 1; n; n *= 2)
  {
    char* p = reinterpret_cast<char*>(mmap(nullptr, n * 2, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));

    if(p == MAP_FAILED)
      return 0;

    u = munmap(p + n, n);
    munmap(p, n * 2);

    if(!u)
      return n;
  }
  return 0;
}
}


#else //============================================================================================
namespace tp_utils
{
//##################################################################################################
inline size_t TP_UTILS_SHARED_EXPORT pageSize()
{
  return 0;
}
}
#endif

#endif
