#pragma once

#include <functional>
#include <mutex>

#include "boost/lockfree/queue.hpp"

struct ExitQueue;

namespace {
std::once_flag initialized;
static std::shared_ptr<ExitQueue> sExitQueue = nullptr;
}

struct ExitQueue {
    typedef std::function<void()> Job;
    boost::lockfree::queue<Job*> jobs;

    ExitQueue() : jobs( 0 ) {}

    // add job, which is executed with call()
    static void add( const Job& job );
    static void initialize();

    // execute all jobs
    static void call();
};

void ExitQueue::add( const ExitQueue::Job& job ) {
    std::call_once( initialized, [] {
        sExitQueue = std::make_shared<ExitQueue>();
    } );

    sExitQueue->jobs.push( new Job( job ) );
}

void ExitQueue::call() {
    if( !sExitQueue ) { return; }

    Job* job = nullptr;

    while( sExitQueue->jobs.pop( job ) ) {
        if( job ) {
            if( *job ) {
                ( *job )();
            }

            delete job;
        }
    }
}
