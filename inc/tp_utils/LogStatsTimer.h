#ifndef tp_utils_LogStatsTimer_h
#define tp_utils_LogStatsTimer_h

#include "tp_utils/detail/log_stats/impl.h"
#include "tp_utils/detail/log_stats/function_time.h"
#include "tp_utils/detail/log_stats/mutex_time.h"
#include "tp_utils/detail/log_stats/ref_count.h"
#include "tp_utils/detail/log_stats/virtual_memory.h"

#include <functional>
#include <thread>

namespace tp_utils
{

//##################################################################################################
//! Base class for periodically logging stats to file or console.
using LogStatsTimer = detail::LogStatsTimer;

//##################################################################################################
//! Log key value stats to file or console.
/*!
\code{.cpp}
//main.cpp

int64_t intervalMS=2000;

// Log to file.
KeyValueLogStatsTimer logStatsTimer("path.log", intervalMS);

// Or to console.
KeyValueLogStatsTimer logStatsTimer(intervalMS);

// Add propducers for the stats that you want to log.
addFunctionTimeStatsProducer(logStatsTimer);
addRefCountStatsProducer(logStatsTimer);
addMemoryUsageProducer(logStatsTimer);

\endcode
*/
using KeyValueLogStatsTimer = detail::KeyValueLogStatsTimer;


#ifdef TP_ENABLE_FUNCTION_TIME
//##################################################################################################
//! Periodically log function time stats to file, overwriting the contents of the file each time.
using SaveFunctionTimeStatsTimer = detail::SaveFunctionTimeStatsTimer;

//##################################################################################################
//! Add function times to the key value logs.
inline void addFunctionTimeProducer(KeyValueLogStatsTimer& keyValueStatsTimer);
#endif


#ifdef TP_ENABLE_MUTEX_TIME
//##################################################################################################
//! Periodically log mutex stats to file, overwriting the contents of the file each time.
using SaveLockStatsTimer = detail::SaveLockStatsTimer;
#endif



#ifdef TP_REF_COUNT
//##################################################################################################
//! Periodically log ref count stats to file, overwriting the contents of the file each time.
using SaveRefCountTimer = detail::SaveRefCountTimer;

//##################################################################################################
//! Add ref counts to the key value logs.
inline void addRefCountProducer(detail::KeyValueLogStatsTimer& keyValueStatsTimer);
#endif



//##################################################################################################
//! Add memory usage to the key value logs.
inline void addMemoryUsageProducer(detail::KeyValueLogStatsTimer& keyValueStatsTimer);

}

#endif

