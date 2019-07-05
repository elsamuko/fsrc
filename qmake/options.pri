
# on macOS, use own threadpool
macx:  DEFINES += 'BOOST_THREADPOOL=0'
# else use boost::asio
!macx: DEFINES += 'BOOST_THREADPOOL=1'

DEFINES += 'THREADED_THREADPOOL=1'  # if 0, use no threadpool at all
DEFINES += 'DETAILED_STATS=1'       # if 1, print detailed times
DEFINES += 'WITH_SSE=1'             # if 1, use mischasan's sse optimized string search
