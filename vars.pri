TARGET = tp_utils
TEMPLATE = lib

DEFINES += TP_UTILS_LIBRARY

SOURCES += src/Globals.cpp
HEADERS += inc/tp_utils/Globals.h

SOURCES += src/FileUtils.cpp
HEADERS += inc/tp_utils/FileUtils.h

SOURCES += src/JSONUtils.cpp
HEADERS += inc/tp_utils/JSONUtils.h

SOURCES += src/StringID.cpp
HEADERS += inc/tp_utils/StringID.h

SOURCES += src/StringIDManager.cpp
HEADERS += inc/tp_utils/StringIDManager.h

SOURCES += src/RefCount.cpp
HEADERS += inc/tp_utils/RefCount.h

SOURCES += src/StackTrace.cpp
HEADERS += inc/tp_utils/StackTrace.h

SOURCES += src/MutexUtils.cpp
HEADERS += inc/tp_utils/MutexUtils.h

SOURCES += src/DebugUtils.cpp
HEADERS += inc/tp_utils/DebugUtils.h

SOURCES += src/TimeUtils.cpp
HEADERS += inc/tp_utils/TimeUtils.h

SOURCES += src/TimerThread.cpp
HEADERS += inc/tp_utils/TimerThread.h

SOURCES += src/AbstractCrossThreadCallback.cpp
HEADERS += inc/tp_utils/AbstractCrossThreadCallback.h

SOURCES += src/AbstractTimerCallback.cpp
HEADERS += inc/tp_utils/AbstractTimerCallback.h

SOURCES += src/Progress.cpp
HEADERS += inc/tp_utils/Progress.h

SOURCES += src/Resources.cpp
HEADERS += inc/tp_utils/Resources.h

SOURCES += src/SignalHandler.cpp
HEADERS += inc/tp_utils/SignalHandler.h

SOURCES += src/Translation.cpp
HEADERS += inc/tp_utils/Translation.h

HEADERS += inc/tp_utils/Parallel.h

HEADERS += inc/tp_utils/CallbackCollection.h

HEADERS += inc/tp_utils/Interface.h

HEADERS += inc/tp_utils/TPPixel.h

HEADERS += inc/tp_utils/PageSize.h

HEADERS += inc/tp_utils/LogStatsTimer.h
HEADERS += inc/tp_utils/detail/log_stats/impl.h
HEADERS += inc/tp_utils/detail/log_stats/function_time.h
HEADERS += inc/tp_utils/detail/log_stats/mutex_time.h
HEADERS += inc/tp_utils/detail/log_stats/ref_count.h
HEADERS += inc/tp_utils/detail/log_stats/virtual_memory.h

HEADERS += inc/tp_utils/ExtendArgs.h

HEADERS += inc/tp_utils/Test.h


