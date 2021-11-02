#define BOOST_TEST_MODULE Utils

#include <boost/test/unit_test.hpp>

#include "utils.hpp"
#include "globmatcher.hpp"

#include <fstream>

BOOST_AUTO_TEST_CASE( Test_isTextFile ) {

    std::string_view pdf( "%PDF", 4 );
    BOOST_CHECK( !utils::isTextFile( pdf ) );

    std::string_view binary( "\0\0", 2 );
    BOOST_CHECK( !utils::isTextFile( binary ) );

    std::string_view text( "Text File\n", 10 );
    BOOST_CHECK( utils::isTextFile( text ) );
}

BOOST_AUTO_TEST_CASE( Test_printFunc ) {
    FILE* cache = fopen( "stdout.tmp", "w" );
    utils::setTarget( cache );
    {
        utils::printFunc( Color::Red, "Red" )();
        utils::printFunc( Color::Green, "Green" )();
        utils::printFunc( Color::Blue, "Blue" )();
        utils::printFunc( Color::Neutral, "\n" )();
    }
    utils::setTarget( stdout );
    fclose( cache );

    utils::FileView content = utils::fromFileP( "stdout.tmp" );

    BOOST_TEST( "\033[1;31mRed\033[0m\033[1;32mGreen\033[0m\033[1;34mBlue\033[0m\n" == content.content );
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

BOOST_AUTO_TEST_CASE( Test_recurseDir ) {

    fs::path dir = fs::temp_directory_path( ) / "test_recurseDir";
    fs::remove_all( dir );
    BOOST_REQUIRE( fs::create_directories( dir ) );

    fs::path test = dir / "test.txt";
    std::string content = "hase";
    { boost::filesystem::ofstream( test ) << content; }

    size_t counter = 0;
    utils::recurseDir( dir.native(), [&]( const sys_string & filename ) {
        ++counter;
        utils::FileView view = utils::fromFileP( filename );
        BOOST_CHECK_EQUAL( std::string( view.content ), content );
    } );


    BOOST_CHECK_EQUAL( counter, 1 );
}

BOOST_AUTO_TEST_CASE( Test_recurseGit ) {

    // must be in within repo
    const fs::path dir = "test_recurseGit";
    fs::remove_all( dir );
    BOOST_REQUIRE( fs::create_directories( dir ) );

    std::string content = "hase";

    for( size_t i = 0; i < 100; ++i ) {
        boost::filesystem::ofstream( dir / utils::format( "test%02d.cpp", i ) ) << content;
    }

    size_t counter = 0;

    utils::gitLsFiles( dir, [&]( const sys_string & filename ) {
        ++counter;
        utils::FileView view = utils::fromFileP( filename );
        BOOST_CHECK_EQUAL( std::string( view.content ), content );
    } );

    BOOST_CHECK_EQUAL( counter, 100 );
}

#if 0
BOOST_AUTO_TEST_CASE( Test_fs ) {

    fs::path pwd = fs::current_path();
    BOOST_CHECK( fs::exists( pwd ) );

    pwd = fs::canonical( pwd );
    BOOST_CHECK( fs::exists( pwd ) );

    pwd = pwd.make_preferred();
    BOOST_CHECK( fs::exists( pwd ) );

}
#endif

BOOST_AUTO_TEST_CASE( Test_absolute ) {
    fs::path pwd = fs::current_path();
    sys_string absolute = utils::absolutePath( pwd.native() );
    sys_string pwd2 = utils::absolutePath();
    BOOST_CHECK( !absolute.empty() );
    BOOST_CHECK( !pwd2.empty() );

#if BOOST_OS_WINDOWS
    std::wcout << absolute << std::endl;
    std::wcout << pwd2 << std::endl;

    sys_string slash     =  utils::absolutePath( L"test/hase/" );
    sys_string backslash = utils::absolutePath( L"test\\hase\\" );
    sys_string noslash   = utils::absolutePath( L"test/hase" );
    std::wcout << slash << std::endl;
    std::wcout << backslash << std::endl;
    std::wcout << noslash << std::endl;
#endif
}

BOOST_AUTO_TEST_CASE( Test_glob ) {
    GlobMatcher matcher( "*.tmp" );
    BOOST_CHECK( matcher.matches( "/hase/blume.tmp" ) );
    BOOST_CHECK( !matcher.matches( "/hase/blume.txt" ) );
    BOOST_CHECK( !matcher.matches( "/hase/blume.tmp.txt" ) );

    GlobMatcher matcher2( "prefix_*.tmp" );
    BOOST_CHECK( matcher2.matches( "/hase/prefix_blume.tmp" ) );
    BOOST_CHECK( !matcher2.matches( "/hase/prefix_blume.txt" ) );
    BOOST_CHECK( !matcher2.matches( "/hase/prefix_blume.tmp.txt" ) );

    GlobMatcher matcher3( "boost*.cmake" );
    BOOST_CHECK( matcher3.matches( "/tmp/cmake/boost.cmake" ) );
}
