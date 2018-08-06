#include <cassert>
#include <chrono>

#include "threadpool.hpp"

// heavily influenced by
// https://github.com/progschj/ThreadPool

ThreadPool::ThreadPool() {
    mRunning.store( true );
    mStopped.store( false );
    mJobCount.store( 0 );

    size_t count = std::thread::hardware_concurrency();
    assert( count > 0 );

    while( count-- ) {
        mWorkers.emplace_back( [this] {
            for( ;; ) {

                if( jobsAvailable() ) {
                    this->workOff();
                } else {
                    this->standInLine();
                }

                if( !( mRunning.load() || mJobCount.load() ) ) {
                    break;
                }

            }
        } );
    }
}

ThreadPool::~ThreadPool() {
    mRunning.store( false );
    waitForAllJobs();

    for( std::thread& worker : mWorkers ) {
        worker.join();
    }
}

void ThreadPool::workOff() {
    Job job;
    {
        std::unique_lock<std::mutex> lock( mJobMutex );

        if( !mJobs.empty() ) {
            job = mJobs.front();
            mJobs.pop();
        }
    }

    if( job ) {
        job();
        // decrement _after_ job is done
        mJobCount--;
    }
}

void ThreadPool::standInLine() {
    std::unique_lock<std::mutex> lock( mWaitingMutex );
    mWaitingCondition.wait_for( lock, std::chrono::milliseconds( 100 ) );
}

bool ThreadPool::jobsAvailable() {
    return mJobCount.load() > 0;
}

bool ThreadPool::add( const Job& job ) {
    if( !mRunning.load() ) {
        return false;
    }

    if( mStopped.load() ) {
        return false;
    }

    std::unique_lock<std::mutex> lock( mJobMutex );
    mJobs.emplace( job );
    mJobCount++;
    mWaitingCondition.notify_one();
    return true;
}

void ThreadPool::waitForAllJobs() {
    mWaitingCondition.notify_all();

    while( mJobCount.load() > 0 ) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        mWaitingCondition.notify_all();
    }
}

void ThreadPool::resume() {
    mStopped.store( false );
}

void ThreadPool::suspend() {

    mStopped.store( true );

    // clear queue
    {
        std::unique_lock<std::mutex> lock( mJobMutex );
        int size = mJobs.size();
        mJobCount -= size;
        std::queue<Job> empty;
        std::swap( mJobs, empty );
    }

    this->waitForAllJobs();
}

int ThreadPool::size() const {
    return mWorkers.size();
}
