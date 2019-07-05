
# on macOS, use own threadpool
macx:  DEFINES += 'THREADPOOL=OWN_THREADPOOL'
# on Linux, use boost::asio
linux: DEFINES += 'THREADPOOL=BOOST_THREADPOOL'
# on Windows, use std::async
win32: DEFINES += 'THREADPOOL=ASYNC_THREADPOOL'

DEFINES += 'DETAILED_STATS=1'       # if 1, print detailed times
DEFINES += 'WITH_SSE=1'             # if 1, use mischasan's sse optimized string search
