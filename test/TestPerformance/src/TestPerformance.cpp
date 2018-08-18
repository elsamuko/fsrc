#define BOOST_TEST_MODULE Performance

#include <boost/test/unit_test.hpp>

#include "utils.hpp"

using fromFileFunc = std::pair<std::string, std::list<std::string_view>>( const sys_string& filename );
size_t run( fromFileFunc fromFile ) {
    size_t sum = 0;
    fs::path include = "/usr/include";

    auto tp = std::chrono::system_clock::now();

    utils::recurseDir( include.native(), [&sum, fromFile]( const sys_string & filename ) {
        auto lines = fromFile( filename );
        sum += lines.first.size();
    } );

    auto duration = std::chrono::system_clock::now() - tp;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>( duration );

    printf( "%lu kB in %lld ms\n", sum / 1024, ms.count() );
    return ms.count();
}

BOOST_AUTO_TEST_SUITE( Performance )

BOOST_AUTO_TEST_CASE( Test_fromFile ) {
    size_t t1 = run( utils::fromFileC );
    size_t t2 = run( utils::fromFile );
    BOOST_CHECK_LT( t1, t2 ); // assume FILE* is faster than std::ifstream
}

BOOST_AUTO_TEST_SUITE_END()

