#define BOOST_TEST_MODULE Performance

#include "boost/iostreams/device/mapped_file.hpp"
#include "boost/test/unit_test.hpp"
#include "boost/asio.hpp"
using boost::asio::post;

#include <fcntl.h>

#if !BOOST_OS_WINDOWS
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "utils.hpp"
#include "PerformanceUtils.hpp"
#include "NewlineParser.hpp"
#include "FileReaders.hpp"

using parseContentFunc = utils::Lines( const char* data, const size_t size, const long long stop );

//! POSIX API with custom parseContent function
utils::FileView fromFileParser( const sys_string& filename, parseContentFunc parse ) {
    utils::FileView view;
    int file = open( filename.c_str(), O_RDONLY | O_BINARY );
    IF_RET( file == -1 );
    utils::ScopeGuard onExit( [file] { close( file ); } );

    view.size = utils::fileSize( file );
    IF_RET( !view.size );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( view.size );

    size_t bytes = _read( file, ptr, view.size );
    IF_RET( view.size != bytes );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min<size_t>( view.size, 100ul ) ) ) );

    view.content = std::string_view( ptr, view.size );
    view.lines = parse( ptr, view.size, view.size );
    return view;
}

utils::FileView fromFileUtils( const sys_string& filename ) {
    return fromFileParser( filename, utils::parseContent );
}

utils::FileView fromFileForLoop( const sys_string& filename ) {
    return fromFileParser( filename, parseContentForLoop );
}

utils::FileView fromFileFind( const sys_string& filename ) {
    return fromFileParser( filename, parseContentFind );
}

utils::FileView fromFileStrchr( const sys_string& filename ) {
    return fromFileParser( filename, parseContentStrchr );
}

utils::FileView fromFileMemchr( const sys_string& filename ) {
    return fromFileParser( filename, parseContentMemchr );
}


std::map<fromFileFunc*, const char*> testsIO = {
    // I/O tests
    {fromFileMmap, "fromFileMmap"},
    {fromFileLocal, "fromFileLocal"},
    {fromFileString, "fromFileString"},
    {fromFileLSeek, "fromFileLSeek"},
    {fromFileTwoFread, "fromFileTwoFread"},
    {fromFileCPP, "fromFileCPP"},
    {utils::fromFileP, "utils::fromFileP"},
#if BOOST_OS_WINDOWS
    {utils::fromWinAPI, "utils::fromWinAPI"},
#endif
};

std::map<fromFileFunc*, const char*> testsNewline = {
    {fromFileUtils, "fromFileUtils"},
    {fromFileForLoop, "fromFileForLoop"},
    {fromFileFind, "fromFileFind"},
    {fromFileStrchr, "fromFileStrchr"},
    {fromFileMemchr, "fromFileMemchr"},
};

// test file I/O
BOOST_AUTO_TEST_CASE( Test_fromFile ) {
    printf( "I/O\n" );
    std::vector<Result> results;

    for( const auto& test : testsIO ) {
        results.emplace_back( run( test.first, test.second ) );
    }

    printSorted( results );
    printf( "\n" );
}

// test newline parsing
BOOST_AUTO_TEST_CASE( Test_parseContent ) {
    printf( "Newline\n" );
    std::vector<Result> results;

    for( const auto& test : testsNewline ) {
        results.emplace_back( run( test.first, test.second ) );
    }

    printSorted( results );
    printf( "\n" );
}

// test printing
BOOST_AUTO_TEST_CASE( Test_printf ) {
    printf( "Output\n" );

    std::string text = "text123";

#if BOOST_OS_WINDOWS
    FILE* file = fopen( L"dump.txt", L"w" );
#else
    FILE* file = fopen( "dump.txt", "w" );
#endif

    auto reset = [file] { fseek( file, 0, SEEK_SET ); };
    std::vector<Result> results = {
        timed1000( "write", [file, text] {
            std::string data = "[" + text + "]\n";
            ( void )!( write( fileno( file ), data.c_str(), data.size() ) );
        }, reset ),
        timed1000( "fprintf", [file, text] {
            fprintf( file, "%s%s]\n", "[", text.c_str() );
        }, reset ),
        timed1000( "fputs", [file, text] {
            fputs( ( "[" + text + "]\n" ).c_str(), file );
        }, reset ),
#ifdef __linux__
        timed1000( "fputs_unlocked", [file, text] {
            fputs_unlocked( ( "[" + text + "]\n" ).c_str(), file );
        }, reset ),
#endif
        timed1000( "fwrite", [file, text] {
            std::string data = "[" + text + "]\n";
            fwrite( data.c_str(), 1, data.size(), file );
        }, reset ),
#ifdef __linux__
        timed1000( "fwrite_unlocked", [file, text] {
            std::string data = "[" + text + "]\n";
            fwrite_unlocked( data.c_str(), 1, data.size(), file );
        }, reset )
#endif
    };

    printSorted( results );
    fclose( file );
    printf( "\n" );
}
