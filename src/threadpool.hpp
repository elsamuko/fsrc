#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <future>

#define NO_THREADPOOL    0
#define OWN_THREADPOOL   1
#define BOOST_THREADPOOL 2
#define ASYNC_THREADPOOL 3

#define QUEUE_TYPE_BOOST  0
#define QUEUE_TYPE_MOODY  1
#define QUEUE_TYPE_ATOMIC 2

#if QUEUE_TYPE == QUEUE_TYPE_BOOST
#include "boost/lockfree/queue.hpp"
#elif QUEUE_TYPE == QUEUE_TYPE_MOODY
#include "concurrentqueue.h"
#elif QUEUE_TYPE == QUEUE_TYPE_ATOMIC
#include "atomic/atomic_queue.h"
#else
#endif
#if THREADPOOL == BOOST_THREADPOOL
#include "boost/asio/thread_pool.hpp"
#include "boost/asio/post.hpp"
#endif // BOOST_THREADPOOL


#if THREADPOOL == NO_THREADPOOL
#define POOL struct { \
    void add( const std::function<void()>& f ) { \
        f(); \
    } \
    } pool;
#endif // NO_THREADPOOL

#if THREADPOOL == BOOST_THREADPOOL
#define POOL struct ThreadPool { \
    boost::asio::thread_pool mPool{ std::min<size_t>( std::thread::hardware_concurrency(), 8u ) }; \
    void add( const std::function<void()>& f ) { \
        boost::asio::post( mPool, f ); \
    } \
    ThreadPool() {} \
    ~ThreadPool() { mPool.join(); } \
} pool;
#endif // BOOST_THREADPOOL

#if THREADPOOL == ASYNC_THREADPOOL
#define POOL struct ThreadPool { \
    std::vector<std::future<void>> results; \
    void add( const std::function<void()>& f ) { \
        results.emplace_back( std::async( std::launch::async, f ) ); \
    } \
    ThreadPool() { results.reserve( 1024 ); } \
} pool;
#endif // BOOST_THREADPOOL

#if THREADPOOL == OWN_THREADPOOL
// max 8 threads, else start/stop needs longer than the actual work
#define POOL ThreadPool pool( std::min<size_t>( std::thread::hardware_concurrency(), 8u ) );
#endif // OWN_THREADPOOL

//! busy waiting lockfree threadpool
//! fast, if you keep the queue filled with jobs
class ThreadPool {
    public:
        typedef std::function<void()> Job;
        ThreadPool( size_t threads );
        ~ThreadPool();
        bool add( const Job& job );
        void join();
    private:
        void initialize();
        void workOff();

#if QUEUE_TYPE == QUEUE_TYPE_BOOST
        boost::lockfree::queue<Job*> jobs;
#elif QUEUE_TYPE == QUEUE_TYPE_MOODY
        moodycamel::ConcurrentQueue<Job> jobs;
#elif QUEUE_TYPE == QUEUE_TYPE_ATOMIC
        atomic_queue::AtomicQueueB<Job*> jobs;
#endif
        size_t threads = 8;
        std::vector<std::thread> workers;
        std::atomic_int count = {0};

        std::once_flag initialized;
        std::atomic_bool running = {true};
};

