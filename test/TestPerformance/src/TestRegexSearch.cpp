#include <regex>

#include "boost/test/unit_test.hpp"
#include "boost/xpressive/xpressive.hpp"
#include "boost/regex.hpp"

#include "PerformanceUtils.hpp"
#include "utils.hpp"
#include "types.hpp"
#include "licence.hpp"

size_t boostRegex( const std::string& content, const std::string& term ) {

    boost::regex regex( term );

    size_t count = 0;

    auto begin = boost::cregex_iterator( &content.front(), 1 + &content.back(), regex );
    auto end   = boost::cregex_iterator();

    for( boost::cregex_iterator match = begin; match != end; ++match ) {
        ++count;
    }

    return count;
}

size_t boostXpressive( const std::string& content, const std::string& term ) {

    boost::xpressive::cregex regex = boost::xpressive::cregex::compile( term.cbegin(), term.cend() );

    size_t count = 0;

    auto begin = boost::xpressive::cregex_iterator( &content.front(), 1 + &content.back(), regex );
    auto end   = boost::xpressive::cregex_iterator();

    for( boost::xpressive::cregex_iterator match = begin; match != end; ++match ) {
        ++count;
    }

    return count;
}

size_t stdRegex( const std::string& content, const std::string& term ) {

    std::regex regex( term );

    size_t count = 0;

    auto begin = std::cregex_iterator( &content.front(), 1 + &content.back(), regex );
    auto end   = std::cregex_iterator();

    for( std::cregex_iterator match = begin; match != end; ++match ) {
        ++count;
    }

    return count;
}

BOOST_AUTO_TEST_CASE( Test_regex ) {
    printf( "Regex search\n" );

    size_t count = 0;
    std::string text( ( const char* )licence, sizeof( licence ) );
    std::string term = "[Ll]icense";

    auto check = [&] {
        // grep -Po '[Ll]icense' < LICENSE | wc -l
        BOOST_CHECK_EQUAL( count, 116 );
    };

#if !BOOST_OS_WINDOWS
    long t_posix = timed1000( "posix", [&text, &term, &count] {
        count = posixRegex( text, term );
    }, check );
#endif

    long t_boost = timed1000( "boost::regex", [&text, &term, &count] {
        count = boostRegex( text, term );
    }, check );

    long t_xpressive = timed1000( "boost::xpressive", [&text, &term, &count] {
        count = boostXpressive( text, term );
    }, check );

    long t_std = timed1000( "std::regex", [&text, &term, &count] {
        count = stdRegex( text, term );
    }, check );

    BOOST_CHECK_GT( t_std, t_boost );
}
