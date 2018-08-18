#define BOOST_TEST_MODULE Utils

#include <boost/test/unit_test.hpp>
#include <fstream>

#include "utils.hpp"

// dummy for Test_printFunc
std::string printed;
int fputs( const char* text, FILE* /*file*/ ) {
    printed.assign( text );
    return printed.size();
}

BOOST_AUTO_TEST_SUITE( Utils )

BOOST_AUTO_TEST_CASE( Test_isTextFile ) {

    std::string_view pdf( "%PDF", 4 );
    BOOST_CHECK( !utils::isTextFile( pdf ) );

    std::string_view binary( "\0\0", 2 );
    BOOST_CHECK( !utils::isTextFile( binary ) );

    std::string_view text( "Text File\n", 10 );
    BOOST_CHECK( utils::isTextFile( text ) );
}

BOOST_AUTO_TEST_CASE( Test_run ) {
    std::list<std::string> res = utils::run( "echo test123" );
    BOOST_REQUIRE( !res.empty() );
    BOOST_CHECK_EQUAL( res.front(), "test123" );
}

BOOST_AUTO_TEST_CASE( Test_printFunc ) {
    std::function<void()> func = utils::printFunc( Color::Neutral, "%s", "test123" );
    func();
    BOOST_CHECK_EQUAL( printed, "test123" );
}

BOOST_AUTO_TEST_SUITE_END()

