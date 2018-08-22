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
        bool add( Job job );
    private:
        void waitForAllJobs();
        void workOff();
        void standInLine();

        std::vector<std::thread> workers;

        std::mutex mutex;
        std::condition_variable wait;

        boost::lockfree::queue<Job*> jobs;
        std::atomic_int count = {0};

        std::atomic_bool running = {true};
};

