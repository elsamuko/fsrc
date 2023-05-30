
# NO_THREADPOOL, run single threaded
# OWN_THREADPOOL, use own busy pool
# BOOST_THREADPOOL, use boost::asio
# ASYNC_THREADPOOL, use std::async

# FIND_MISCHASAN, use mischasan's sse optimized string search
# FIND_SSE_OWN, use own sse optimized string search
# FIND_TRAITS, use traits search from basic_string.tcc
# FIND_STRSTR, use builtin strstr

# QUEUE_TYPE_BOOST, boost::lockfree::queue
# QUEUE_TYPE_MOODY, moodycamel::ConcurrentQueue
# QUEUE_TYPE_ATOMIC, atomic_queue::AtomicQueueB

if(APPLE)
    # on macOS, use own threadpool
    add_definitions(-DTHREADPOOL=OWN_THREADPOOL)
    add_definitions(-DFIND_ALGO=FIND_MISCHASAN)
endif()

if(UNIX AND NOT APPLE)
    # on Linux, use own threadpool
    add_definitions(-DTHREADPOOL=OWN_THREADPOOL)
    add_definitions(-DFIND_ALGO=FIND_MISCHASAN)
endif()

if(WIN32)
    # on Windows, use std::async
    add_definitions(-DTHREADPOOL=ASYNC_THREADPOOL)
    add_definitions(-DFIND_ALGO=FIND_STRSTR)
endif()

add_definitions(-DQUEUE_TYPE=QUEUE_TYPE_MOODY)
add_definitions(-DDETAILED_STATS=1) # if 1, print detailed times
