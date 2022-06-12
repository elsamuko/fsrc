
if(APPLE)
    # on macOS, use own threadpool
    add_definitions(-DTHREADPOOL=OWN_THREADPOOL)
    add_definitions(-DFIND_ALGO=FIND_STRSTR)
endif()

if(UNIX AND NOT APPLE)
    # on Linux, use boost::asio
    add_definitions(-DTHREADPOOL=OWN_THREADPOOL)
    add_definitions(-DFIND_ALGO=FIND_STRSTR)
endif()

if(WIN32)
    # on Windows, use std::async
    add_definitions(-DTHREADPOOL=ASYNC_THREADPOOL)
    add_definitions(-DFIND_ALGO=FIND_STRSTR)
endif()

add_definitions(-DDETAILED_STATS=1) # if 1, print detailed times
