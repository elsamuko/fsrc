
if(APPLE)
    # on macOS, use own threadpool
    add_definitions(-DTHREADPOOL=OWN_THREADPOOL)
endif()

if(UNIX AND NOT APPLE)
    # on Linux, use boost::asio
    add_definitions(-DTHREADPOOL=BOOST_THREADPOOL)
endif()

if(WIN32)
    # on Windows, use std::async
    add_definitions(-DTHREADPOOL=ASYNC_THREADPOOL)
endif()

add_definitions(-DDETAILED_STATS=1) # if 1, print detailed times
add_definitions(-DWITH_SSE=1) # if 1, use mischasan's sse optimized string search
