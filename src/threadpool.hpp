#pragma once

#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>

#include "boost/lockfree/queue.hpp"
#include "boost/asio/thread_pool.hpp"
#include "boost/asio/post.hpp"

#if THREADED_THREADPOOL

#if BOOST_THREADPOOL

#define POOL struct ThreadPool { \
    boost::asio::thread_pool mPool{ std::min<size_t>( std::thread::hardware_concurrency(), 8u ) }; \
    void add( const std::function<void()>& f ) { \
        boost::asio::post( mPool, f ); \
    } \
    ThreadPool() {} \
    ~ThreadPool() { mPool.join(); } \
} pool;

#else

// max 8 threads, else start/stop needs longer than the actual work
#define POOL ThreadPool pool( std::min<size_t>( std::thread::hardware_concurrency(), 8u ) );

#endif // BOOST_THREADPOOL

#else

#define POOL struct { \
    void add( const std::function<void()>& f ) { \
        f(); \
    } \
    } pool;

#endif // THREADED_THREADPOOL

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

        size_t threads = 4;
        std::vector<std::thread> workers;

        boost::lockfree::queue<Job*> jobs;
        std::atomic_int count = {0};

        std::once_flag initialized;
        std::atomic_bool running = {true};
};

