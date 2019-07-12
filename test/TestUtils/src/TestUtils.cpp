#define BOOST_TEST_MODULE Utils

#include <boost/test/unit_test.hpp>
#include <fstream>

#include "utils.hpp"

// dummy for Test_printFunc
std::string printed;
size_t fwrite( const void* ptr, size_t size, size_t count, FILE* /*file*/ ) {
    printed.assign( ( char* ) ptr, size * count );
    return printed.size();
}

#ifdef __linux__
static bool called = false;
int system( const char* command ) {
    return called;
}
#endif

BOOST_AUTO_TEST_CASE( Test_isTextFile ) {

    std::string_view pdf( "%PDF", 4 );
    BOOST_CHECK( !utils::isTextFile( pdf ) );

    std::string_view binary( "\0\0", 2 );
    BOOST_CHECK( !utils::isTextFile( binary ) );

    std::string_view text( "Text File\n", 10 );
    BOOST_CHECK( utils::isTextFile( text ) );
}

BOOST_AUTO_TEST_CASE( Test_run ) {
    std::vector<std::string> res = utils::run( "echo test123" );
    BOOST_REQUIRE( !res.empty() );
    BOOST_CHECK_EQUAL( res.front(), "test123" );
}

BOOST_AUTO_TEST_CASE( Test_printFunc ) {
    std::function<void()> func = utils::printFunc( Color::Neutral, "test123" );
    func();
    BOOST_CHECK_EQUAL( printed, "test123" );
}

BOOST_AUTO_TEST_CASE( Test_parseContent ) {
    auto lines = utils::parseContent( "\n\n", 2, 2 );
    BOOST_CHECK_EQUAL( lines.size(), 2 );

    lines = utils::parseContent( "123", 3, 3 );
    BOOST_CHECK_EQUAL( lines.size(), 1 );

    lines = utils::parseContent( "123\n", 4, 4 );
    BOOST_CHECK_EQUAL( lines.size(), 1 );

    lines = utils::parseContent( "123\n\n", 5, 5 );
    BOOST_CHECK_EQUAL( lines.size(), 2 );

    lines = utils::parseContent( "123\n\n123", 8, 8 );
    BOOST_CHECK_EQUAL( lines.size(), 3 );

    lines = utils::parseContent( "123\n\n123\n", 9, 9 );
    BOOST_CHECK_EQUAL( lines.size(), 3 );

    lines = utils::parseContent( "123\n\n123\n4", 10, 10 );
    BOOST_CHECK_EQUAL( lines.size(), 4 );
}

#ifdef __linux__
BOOST_AUTO_TEST_CASE( Test_openFile ) {
    BOOST_CHECK( utils::openFile( "/usr/include/errno.h" ) );
    BOOST_CHECK( called );
}
#endif
