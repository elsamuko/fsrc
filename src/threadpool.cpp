#include <cassert>
#include <chrono>
#include <mutex>

#include "threadpool.hpp"

// heavily influenced by
// https://github.com/progschj/ThreadPool

ThreadPool::ThreadPool( size_t threads ) : threads( threads ) {}

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

std::once_flag initialized;

bool ThreadPool::add( Job job ) {
    std::call_once( initialized, [this] { this->initialize(); } );
    jobs.push( new Job( std::move( job ) ) );
    count++;
    return true;
}

void ThreadPool::initialize() {
    while( threads-- ) {
        workers.reserve( threads );
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
