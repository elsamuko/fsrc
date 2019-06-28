#include "boost/test/unit_test.hpp"
#include "boost/timer/timer.hpp"
#include "boost/asio.hpp"

#include "threadpool.hpp"

using boost::asio::post;

BOOST_AUTO_TEST_CASE( Test_ThreadPool ) {
    long ns_asio = 0;
    long ns_own = 0;
    long ns_async = 0;
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
    {
        boost::timer::cpu_timer stopwatch;
        stopwatch.start();
        {
            std::vector<std::future<void>> results;
            results.reserve( 1000 );

            for( int i = 0; i < 1000; ++i ) {
                results.emplace_back( std::async( std::launch::async, [&counter] {counter++;} ) );
            }
        }

        stopwatch.stop();
        ns_async = stopwatch.elapsed().wall;
    }
    BOOST_REQUIRE_EQUAL( counter, 3000 );

    printf( "Threadpool\n" );
    printf( "              own : %6ld us\n", ns_own / 1000 );
    printf( "            boost : %6ld us\n", ns_asio / 1000 );
    printf( "            async : %6ld us\n\n", ns_async / 1000 );
    BOOST_CHECK_LT( ns_asio / 1000, ns_own / 1000 ); // assume own tp is slower than boost::asio
}
