#include <chrono>

#include "threadpool.hpp"

// heavily influenced by
// https://github.com/progschj/ThreadPool

ThreadPool::ThreadPool( size_t threads ) : threads( threads ), jobs( 0 ) {}

ThreadPool::~ThreadPool() {
    join();
}

#if QUEUE_TYPE == QUEUE_TYPE_BOOST
void ThreadPool::workOff() {
    Job* job = nullptr;

    if( jobs.pop( job ) && job && *job ) {
        ( *job )();
        // decrement _after_ job is done
        count--;
        delete job;
    }
}

bool ThreadPool::add( const Job& job ) {
    std::call_once( initialized, [this] { this->initialize(); } );
    jobs.push( new Job( job ) );
    count++;
    return true;
}
#else
void ThreadPool::workOff() {
    Job job;

    if( jobs.try_dequeue( job ) && job ) {
        job();
        // decrement _after_ job is done
        count--;
    }
}

bool ThreadPool::add( const Job& job ) {
    std::call_once( initialized, [this] { this->initialize(); } );
    jobs.enqueue( job );
    count++;
    return true;
}
#endif

void ThreadPool::join() {
    if( running ) {
        running = false;

        for( std::thread& worker : workers ) {
            worker.join();
        }
    }
}

void ThreadPool::initialize() {
    while( threads-- ) {
        workers.reserve( threads );
        workers.emplace_back( [this] {
            for( ;; ) {

                if( count ) {
                    this->workOff();
                } else {
                    std::this_thread::sleep_for( std::chrono::microseconds( 1 ) );
                }

                if( !( running || count ) ) {
                    break;
                }
            }
        } );
    }
}
