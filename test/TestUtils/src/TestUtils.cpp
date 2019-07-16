#define BOOST_TEST_MODULE Utils

#include <boost/test/unit_test.hpp>
#include <fstream>

#include "utils.hpp"

BOOST_AUTO_TEST_CASE( Test_isTextFile ) {

    std::string_view pdf( "%PDF", 4 );
    BOOST_CHECK( !utils::isTextFile( pdf ) );

    std::string_view binary( "\0\0", 2 );
    BOOST_CHECK( !utils::isTextFile( binary ) );

    std::string_view text( "Text File\n", 10 );
    BOOST_CHECK( utils::isTextFile( text ) );
}

BOOST_AUTO_TEST_CASE( Test_printFunc ) {
    utils::printFunc( Color::Red, "Red" )();
    utils::printFunc( Color::Green, "Green" )();
    utils::printFunc( Color::Blue, "Blue" )();
    utils::printFunc( Color::Neutral, "\n" )();
    BOOST_CHECK( true );
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

BOOST_AUTO_TEST_CASE( Test_openFile ) {
    BOOST_CHECK( utils::openFile( "/usr/include/errno.h" ) );
#ifdef __linux__
    BOOST_CHECK( called );
#endif
}
