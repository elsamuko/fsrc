
# NO_THREADPOOL, run single threaded
# OWN_THREADPOOL, use own busy pool
# BOOST_THREADPOOL, use boost::asio
# ASYNC_THREADPOOL, use std::async
macx:  DEFINES += 'THREADPOOL=OWN_THREADPOOL'
linux: DEFINES += 'THREADPOOL=OWN_THREADPOOL'
win32: DEFINES += 'THREADPOOL=ASYNC_THREADPOOL'

# FIND_MISCHASAN, use mischasan's sse optimized string search
# FIND_SSE_OWN, use own sse optimized string search
# FIND_TRAITS, use traits search from basic_string.tcc
# FIND_STRSTR, use builtin strstr
macx:  DEFINES += 'FIND_ALGO=FIND_STRSTR'
linux: DEFINES += 'FIND_ALGO=FIND_STRSTR'
win32: DEFINES += 'FIND_ALGO=FIND_STRSTR'

DEFINES += 'DETAILED_STATS=1'       # if 1, print detailed times
