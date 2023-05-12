#include "boost/test/unit_test.hpp"

#include "boost/lockfree/queue.hpp"
#include "concurrentqueue.h"

#include "boost/asio/thread_pool.hpp"
#include "boost/asio/post.hpp"

BOOST_AUTO_TEST_CASE( Test_queue ) {
    typedef std::function<void()> Job;
    boost::lockfree::queue<Job*> jobs_boost( 0 );
    moodycamel::ConcurrentQueue<Job> jobs_moody;

    bool stop = false;
    std::atomic_size_t counter = 0;

    boost::asio::thread_pool pool{ 8u };

    for( int i = 0; i < 4; ++i ) {
        boost::asio::post( pool, [&] {
            while( !stop ) {
                Job* job = nullptr;

                if( jobs_boost.pop( job ) && job && *job ) {
                    ( *job )();
                    delete job;
                }
            }
        } );
    }

    for( int i = 0; i < 4; ++i ) {
        boost::asio::post( pool, [&] {
            while( !stop ) {
                jobs_boost.push( new Job( [&] {
                    counter++;
                } ) );
            }
        } );
    }

    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    stop = true;
    pool.join();

    printf( "boost: %zu\n", counter.load() );

    stop = false;
    counter = 0;

    boost::asio::thread_pool pool2{ 8u };

    for( int i = 0; i < 4; ++i ) {
        boost::asio::post( pool2, [&] {
            while( !stop ) {
                Job job;

                if( jobs_moody.try_dequeue( job ) && job ) {
                    job();
                }
            }
        } );
    }


    for( int i = 0; i < 4; ++i ) {
        boost::asio::post( pool2, [&] {
            while( !stop ) {
                jobs_moody.enqueue( [&] {
                    counter++;
                } );
            }
        } );
    }

    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    stop = true;
    pool2.join();

    printf( "moody: %zu\n", counter.load() );
}
