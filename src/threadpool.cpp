#include <chrono>

#include "threadpool.hpp"

// heavily influenced by
// https://github.com/progschj/ThreadPool

ThreadPool::ThreadPool( size_t threads ) : jobs( 0 ), threads( threads ) {}

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

bool ThreadPool::add( Job job ) {
    std::call_once( initialized, [this] { this->initialize(); } );
    jobs.push( new Job( std::move( job ) ) );
    count++;
    return true;
}

bool ThreadPool::empty() const {
    return jobs.empty();
}
#elif QUEUE_TYPE == QUEUE_TYPE_MOODY
void ThreadPool::workOff() {
    Job job;

    if( jobs.try_dequeue( job ) && job ) {
        job();
        // decrement _after_ job is done
        count--;
    }
}

bool ThreadPool::add( Job job ) {
    std::call_once( initialized, [this] { this->initialize(); } );
    jobs.enqueue( std::move( job ) );
    count++;
    return true;
}

bool ThreadPool::empty() const {
    return jobs.size_approx() == 0;
}
#elif QUEUE_TYPE == QUEUE_TYPE_ATOMIC
void ThreadPool::workOff() {
    Job* job = nullptr;

    if( jobs.try_pop( job ) && job && *job ) {
        ( *job )();
        // decrement _after_ job is done
        count--;
        delete job;
    }
}

bool ThreadPool::add( Job job ) {
    std::call_once( initialized, [this] { this->initialize(); } );
    jobs.push( new Job( std::move( job ) ) );
    count++;
    return true;
}

bool ThreadPool::empty() const {
    return jobs.was_empty();
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
    workers.reserve( threads );

    while( threads-- ) {
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
