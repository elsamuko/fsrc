#include "boost/test/unit_test.hpp"
#include "boost/algorithm/searching/boyer_moore_horspool.hpp"
#include "boost/algorithm/searching/knuth_morris_pratt.hpp"

#include "PerformanceUtils.hpp"
#include "licence.hpp"

BOOST_AUTO_TEST_CASE( Test_find ) {
    printf( "String search\n" );
    std::string text( ( const char* )licence, sizeof( licence ) );
    std::string term = "Termination";
    size_t pos = 0;
    char* ptr = nullptr;
    std::string::const_iterator it;
    std::string::const_iterator it2;
    boost::algorithm::boyer_moore_horspool bmh( term.begin(), term.end() );
    boost::algorithm::knuth_morris_pratt kmp( term.begin(), term.end() );

    boost::int_least64_t t_find = timed1000( "find", [&text, &term, &pos] {
        pos = text.find( term );
    } );

#if !BOOST_OS_WINDOWS
    boost::int_least64_t t_memmem = timed1000( "memmem", [&text, &term, &ptr] {
        ptr = ( char* )memmem( text.data(), text.size(), term.data(), term.size() );
    } );
#endif

    boost::int_least64_t t_strstr = timed1000( "strstr", [&text, &term, &ptr] {
        ptr = strstr( text.data(), term.data() );
    } );

    boost::int_least64_t t_BMH = timed1000( "BMH search", [&text, &it, &bmh] {
        it = bmh( text.cbegin(), text.cend() ).first;
    } );

    boost::int_least64_t t_KMP = timed1000( "KMP search", [&text, &it2, &kmp] {
        it2 = kmp( text.cbegin(), text.cend() ).first;
    } );

    BOOST_REQUIRE_NE( pos, std::string::npos );
    BOOST_REQUIRE_NE( ptr, nullptr );
    BOOST_REQUIRE_EQUAL( ptr - text.data(), it - text.cbegin() );

    BOOST_CHECK_LT( t_find, t_strstr ); // assume find is faster than memmem
    BOOST_CHECK_GT( t_BMH, t_find ); // assume bmh is slower than find
    printf( "\n" );
}
