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
                    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
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

bool ThreadPool::add( Job job ) {
    jobs.push( new Job( std::move( job ) ) );
    count++;
    return true;
}
