#include "boost/test/unit_test.hpp"
#include "boost/asio.hpp"

#include "threadpool.hpp"
#include "stopwatch.hpp"

using boost::asio::post;

BOOST_AUTO_TEST_CASE( Test_ThreadPool ) {
    long ns_asio = 0;
    long ns_own = 0;
    long ns_async = 0;
    STOPWATCH

    std::atomic_int counter = 0;
    {
        START {
            boost::asio::thread_pool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

            for( int i = 0; i < 1000; ++i ) {
                boost::asio::post( pool, [&counter] {counter++;} );
            }

            pool.join();
        }

        STOP( ns_asio );
    }
    BOOST_REQUIRE_EQUAL( counter, 1000 );
    {
        START {
            ThreadPool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

            for( int i = 0; i < 1000; ++i ) {
                pool.add( [&counter] {counter++;} );
            }
        }

        STOP( ns_own )
    }
    BOOST_REQUIRE_EQUAL( counter, 2000 );
    {
        START {
            std::vector<std::future<void>> results;
            results.reserve( 1000 );

            for( int i = 0; i < 1000; ++i ) {
                results.emplace_back( std::async( std::launch::async, [&counter] {counter++;} ) );
            }
        }

        STOP( ns_async )
    }
    BOOST_REQUIRE_EQUAL( counter, 3000 );

    printf( "Threadpool\n" );
    printf( "              own : %6ld us\n", ns_own / 1000 );
    printf( "            boost : %6ld us\n", ns_asio / 1000 );
    printf( "            async : %6ld us\n\n", ns_async / 1000 );
    BOOST_CHECK_LT( ns_asio / 1000, ns_own / 1000 ); // assume own tp is slower than boost::asio
}
