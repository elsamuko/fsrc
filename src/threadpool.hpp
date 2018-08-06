#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <condition_variable>


class ThreadPool {
    public:
        typedef std::function<void()> Job;
        ThreadPool();
        ~ThreadPool();
        bool add( const ThreadPool::Job& job );
        void waitForAllJobs();
        void suspend();
        void resume();
        int size() const;
    private:
        void workOff();
        void standInLine();
        bool jobsAvailable();

        std::vector<std::thread> mWorkers;

        std::mutex mWaitingMutex;
        std::condition_variable mWaitingCondition;

        mutable std::mutex mJobMutex;
        std::queue<Job> mJobs;
        std::atomic_int mJobCount;

        std::atomic_bool mRunning;
        std::atomic_bool mStopped;
};

