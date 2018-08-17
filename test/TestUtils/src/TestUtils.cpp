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

    std::ofstream of( "sample.pdf", std::ios::out | std::ios::binary );
    of.write( "%PDF", 4 );
    of.close();

    BOOST_CHECK( !utils::isTextFile( "sample.pdf" ) );

    std::ofstream of2( "sample.bin", std::ios::out | std::ios::binary );
    of2.write( "\0\0", 2 );
    of2.close();

    BOOST_CHECK( !utils::isTextFile( "sample.bin" ) );

    std::ofstream of3( "sample.txt", std::ios::out );
    of3 << "Text File" << std::endl;
    of3.close();

    BOOST_CHECK( utils::isTextFile( "sample.txt" ) );
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

