#include "boost/test/unit_test.hpp"
#include "boost/algorithm/searching/boyer_moore_horspool.hpp"
#include "boost/algorithm/searching/knuth_morris_pratt.hpp"

#include "PerformanceUtils.hpp"
#include "licence.hpp"
#include "mischasan.hpp"
#include "stdstr.hpp"
#include "ssefind.hpp"

BOOST_AUTO_TEST_CASE( Test_find ) {
    printf( "String search\n" );

    std::string term = "t4tb7qfSFb2";
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
        std::string_view view( text );

        // init with correct values
        size_t pos = std::string::npos;
        const char* ptr = nullptr;
        std::string::const_iterator it = text.cend();

        boost::algorithm::boyer_moore_horspool bmh( term.begin(), term.end() );
        boost::algorithm::knuth_morris_pratt kmp( term.begin(), term.end() );

        auto checks = [&] {
            BOOST_REQUIRE_EQUAL( pos, std::string::npos );
            BOOST_REQUIRE_EQUAL( ptr, nullptr );
            BOOST_REQUIRE_EQUAL( it - text.cbegin(), text.size() );
        };

        long t_find = timed1000( "find", [&text, &term, &pos] {
            pos = text.find( term );
        }, checks );

        long t_std = timed1000( "fromStd", [&text, &term, &ptr] {
            ptr = fromStd::strstr( text.data(), text.size(), term.data(), term.size() );
        }, checks );

        long t_view = timed1000( "view", [&view, &term, &pos] {
            pos = view.find( term );
        }, checks );

#if !BOOST_OS_WINDOWS
        long t_memmem = timed1000( "memmem", [&text, &term, &ptr] {
            ptr = ( char* )memmem( text.data(), text.size(), term.data(), term.size() );
        }, checks );

        long t_mischasan = timed1000( "mischasan", [&text, &term, &ptr] {
            ptr = mischasan::scanstrN( text.data(), text.size(), term.data(), term.size() );
        }, checks );

        long t_sseown = timed1000( "sseown", [&text, &term, &ptr] {
            auto v = sse::find( text.data(), text.size(), term.data(), term.size() );
            ptr = nullptr;
        }, checks );
#endif

        long t_strstr = timed1000( "strstr", [&text, &term, &ptr] {
            ptr = strstr( text.data(), term.data() );
        }, checks );

        long t_BMH = timed1000( "BMH search", [&text, &it, &bmh] {
            it = bmh( text.cbegin(), text.cend() ).first;
        }, checks );

        long t_KMP = timed1000( "KMP search", [&text, &it, &kmp] {
            it = kmp( text.cbegin(), text.cend() ).first;
        }, checks );


        BOOST_CHECK_GT( t_find, t_strstr ); // assume find is slower than strstr
        BOOST_CHECK_LT( t_BMH, t_find ); // assume bmh is slower than find
        printf( "\n" );
    }
}
