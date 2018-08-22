#include <cassert>
#include <chrono>

#include "threadpool.hpp"

// heavily influenced by
// https://github.com/progschj/ThreadPool

ThreadPool::ThreadPool( size_t threads ) {
    while( threads-- ) {
        workers.emplace_back( [this] {
            for( ;; ) {

                if( count ) {
                    this->workOff();
                } else {
                    standInLine();
                }

                if( !( running || count ) ) {
                    break;
                }
            }
        } );
    }
}

ThreadPool::~ThreadPool() {
    running = false;
    waitForAllJobs();

    for( std::thread& worker : workers ) {
        worker.join();
    }
}

void ThreadPool::workOff() {
    Job* job = nullptr;

    if( jobs.pop( job ) && job && *job ) {
        ( *job )();
        // decrement _after_ job is done
        count--;
        delete job;
    }
}

void ThreadPool::standInLine() {
    std::unique_lock<std::mutex> lock( mutex );
    wait.wait_for( lock, std::chrono::milliseconds( 10 ) );
}

bool ThreadPool::add( Job job ) {
    jobs.push( new Job( std::move( job ) ) );
    count++;
    wait.notify_one();
    return true;
}

void ThreadPool::waitForAllJobs() {
    wait.notify_all();

    while( count > 0 ) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        wait.notify_all();
    }
}
