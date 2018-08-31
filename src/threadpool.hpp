#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <condition_variable>

#include <boost/lockfree/queue.hpp>

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

