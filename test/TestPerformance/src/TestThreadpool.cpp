#include "boost/test/unit_test.hpp"
#include "boost/timer/timer.hpp"
#include "boost/asio.hpp"

#include "threadpool.hpp"

using boost::asio::post;

BOOST_AUTO_TEST_CASE( Test_ThreadPool ) {
    boost::int_least64_t ns_asio = 0;
    boost::int_least64_t ns_own = 0;
    std::atomic_int counter = 0;
    {
        boost::timer::cpu_timer stopwatch;
        stopwatch.start();
        {
            boost::asio::thread_pool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

            for( int i = 0; i < 1000; ++i ) {
                boost::asio::post( pool, [&counter] {counter++;} );
            }

            pool.join();
        }

        stopwatch.stop();
        ns_asio = stopwatch.elapsed().wall;
    }
    BOOST_REQUIRE_EQUAL( counter, 1000 );
    {
        boost::timer::cpu_timer stopwatch;
        stopwatch.start();
        {
            ThreadPool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

            for( int i = 0; i < 1000; ++i ) {
                pool.add( [&counter] {counter++;} );
            }
        }

        stopwatch.stop();
        ns_own = stopwatch.elapsed().wall;
    }
    BOOST_REQUIRE_EQUAL( counter, 2000 );

    printf( "Threadpool\n" );
    printf( "              own : %5llu us\n", ns_own / 1000 );
    printf( "            boost : %5llu us\n\n", ns_asio / 1000 );
    BOOST_CHECK_LT( ns_asio, ns_own ); // assume own tp is slower than boost::asio
}
