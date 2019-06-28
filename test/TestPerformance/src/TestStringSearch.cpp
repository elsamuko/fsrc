#include "boost/test/unit_test.hpp"
#include "boost/algorithm/searching/boyer_moore_horspool.hpp"
#include "boost/algorithm/searching/knuth_morris_pratt.hpp"

#include "PerformanceUtils.hpp"
#include "licence.hpp"

BOOST_AUTO_TEST_CASE( Test_find ) {
    printf( "String search\n" );

    std::string term = "Termination";
    std::string full( ( const char* )licence, sizeof( licence ) );
    std::string small = R"gpl(
Moreover, your license from a particular copyright holder is
reinstated permanently if the copyright holder notifies you of the
violation by some reasonable means, this is the first time you have
received notice of violation of this License (for any work) from that
copyright holder, and you cure the violation prior to 30 days after
your receipt of the notice.

  Termination of your rights under this section does not terminate the
licenses of parties who have received copies or rights from you under
this License.  If your rights have been terminated and not permanently
reinstated, you do not qualify to receive new licenses for the same
material under section 10.
)gpl";

    for( std::string text : { small, full } ) {
        size_t pos = 0;
        char* ptr = nullptr;
        std::string::const_iterator it;
        std::string::const_iterator it2;
        boost::algorithm::boyer_moore_horspool bmh( term.begin(), term.end() );
        boost::algorithm::knuth_morris_pratt kmp( term.begin(), term.end() );

        long t_find = timed1000( "find", [&text, &term, &pos] {
            pos = text.find( term );
        } );

#if !BOOST_OS_WINDOWS
        long t_memmem = timed1000( "memmem", [&text, &term, &ptr] {
            ptr = ( char* )memmem( text.data(), text.size(), term.data(), term.size() );
        } );
#endif

        long t_strstr = timed1000( "strstr", [&text, &term, &ptr] {
            ptr = strstr( text.data(), term.data() );
        } );

        long t_BMH = timed1000( "BMH search", [&text, &it, &bmh] {
            it = bmh( text.cbegin(), text.cend() ).first;
        } );

        long t_KMP = timed1000( "KMP search", [&text, &it2, &kmp] {
            it2 = kmp( text.cbegin(), text.cend() ).first;
        } );

        BOOST_REQUIRE_NE( pos, std::string::npos );
        BOOST_REQUIRE_NE( ptr, nullptr );
        BOOST_REQUIRE_EQUAL( ptr - text.data(), it - text.cbegin() );

#if BOOST_OS_WINDOWS
        BOOST_CHECK_GT( t_find, t_strstr ); // assume find is slower than strstr on Windows
#else
        BOOST_CHECK_LT( t_find, t_strstr ); // assume find is faster than strstr
#endif
        BOOST_CHECK_GT( t_BMH, t_find ); // assume bmh is slower than find
        printf( "\n" );
    }
}
