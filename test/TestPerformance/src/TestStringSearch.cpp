#include "boost/test/unit_test.hpp"
#include "boost/algorithm/searching/boyer_moore_horspool.hpp"
#include "boost/algorithm/searching/knuth_morris_pratt.hpp"

#include <string>
#include <functional>

#include "PerformanceUtils.hpp"
#include "licence.hpp"
#include "mischasan.hpp"
#include "stdstr.hpp"
#include "ssefind.hpp"

BOOST_AUTO_TEST_CASE( Test_find ) {
    printf( "String search\n" );

    std::string term = "t4tb7qfSFb2";
    std::string largeText( ( const char* )licence, sizeof( licence ) );
    std::string smallText = "Moreover, your license from a particular copyright holder is\n"
                            "reinstated permanently if the copyright holder notifies you of the\n"
                            "violation by some reasonable means, this is the first time you have\n"
                            "received notice of violation of this License (for any work) from that\n"
                            "copyright holder, and you cure the violation prior to 30 days after\n"
                            "your receipt of the notice.\n"
                            "\n"
                            "  Termination of your rights under this section does not terminate the\n"
                            "licenses of parties who have received copies or rights from you under\n"
                            "this License.  If your rights have been terminated and not permanently\n"
                            "reinstated, you do not qualify to receive new licenses for the same\n"
                            "material under section 10.\n";

    for( const std::string& text : { smallText, largeText } ) {
        std::string_view view( text );

        // init with correct values
        size_t pos = std::string::npos;
        const char* ptr = nullptr;
        std::string::const_iterator it = text.cend();

        boost::algorithm::boyer_moore_horspool bmh( term.begin(), term.end() );
        boost::algorithm::knuth_morris_pratt kmp( term.begin(), term.end() );
        std::boyer_moore_searcher bms( term.begin(), term.end() );
        std::boyer_moore_horspool_searcher bmhs( term.begin(), term.end() );

        auto checks = [&] {
            BOOST_REQUIRE_EQUAL( pos, std::string::npos );
            BOOST_REQUIRE_EQUAL( ptr, nullptr );
            BOOST_REQUIRE_EQUAL( it - text.cbegin(), text.size() );
        };

        std::vector<Result> results = {
            timed1000( "find", [&text, &term, &pos] {
                pos = text.find( term );
            }, checks ),

            timed1000( "fromStd", [&text, &term, &ptr] {
                ptr = fromStd::strstr( text.data(), text.size(), term.data(), term.size() );
            }, checks ),

            timed1000( "view", [&view, &term, &pos] {
                pos = view.find( term );
            }, checks ),

#if !BOOST_OS_WINDOWS
            timed1000( "memmem", [&text, &term, &ptr] {
                ptr = ( char* )memmem( text.data(), text.size(), term.data(), term.size() );
            }, checks ),

            timed1000( "mischasan", [&text, &term, &ptr] {
                ptr = mischasan::scanstrN( text.data(), text.size(), term.data(), term.size() );
            }, checks ),
            timed1000( "sseown", [&view, &term, &ptr] {
                auto v = sse::find( view, term );
                ptr = nullptr;
            }, checks ),
#endif

            timed1000( "strstr", [&text, &term, &ptr] {
                ptr = strstr( text.data(), term.data() );
            }, checks ),

            timed1000( "BMH search", [&text, &it, &bmh] {
                it = bmh( text.cbegin(), text.cend() ).first;
            }, checks ),

            timed1000( "KMP search", [&text, &it, &kmp] {
                it = kmp( text.cbegin(), text.cend() ).first;
            }, checks ),

            timed1000( "BMS search", [&text, &it, &bms] {
                it = bms( text.cbegin(), text.cend() ).first;
            }, checks ),

            timed1000( "BMHS search", [&text, &it, &bmhs] {
                it = bmhs( text.cbegin(), text.cend() ).first;
            }, checks ),

        };

        printSorted( results );
        printf( "\n" );
    }
}
