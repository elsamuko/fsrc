#define BOOST_TEST_MODULE Performance

#include "boost/test/unit_test.hpp"
#include "boost/timer/timer.hpp"
#include "boost/asio.hpp"
using boost::asio::post;

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "utils.hpp"
#include "threadpool.hpp"

utils::Lines parseContentForLoop( const char* data, const size_t size ) {
    utils::Lines lines;
    lines.reserve( 128 );

    if( size == 0 ) { return lines; }

    char* c_old = ( char* )data;
    char* c_new = c_old;
    char* c_end = c_old + size;

    for( ; *c_new; ++c_new ) {
        // just skip windows line endings
        if( *c_new == '\r' ) {
            ++c_new;
        }

        if( *c_new == '\n' ) {
            lines.emplace_back( c_old, c_new - c_old );
            c_old = c_new + 1;
        }
    }

    if( c_old != c_end ) {
        lines.emplace_back( c_old, c_end - c_old );
    }

    lines.shrink_to_fit();
    return lines;
}

utils::Lines parseContentFind( const char* data, const size_t size ) {
    utils::Lines lines;
    lines.reserve( 128 );

    if( size == 0 ) { return lines; }

    std::string_view view( data, size );
    size_t pos_old = 0;
    size_t pos_new = view.find( '\n' );

    while( pos_new != std::string::npos ) {
        lines.emplace_back( data + pos_old, pos_new - pos_old );
        pos_old = pos_new + 1;
        pos_new = view.find( '\n', pos_old );
    }

    if( pos_old != size ) {
        lines.emplace_back( data + pos_old, size - pos_old );
    }

    lines.shrink_to_fit();
    return lines;
}

using parseContentFunc = utils::Lines( const char* data, const size_t size );

//! POSIX API with custom parseContent function
std::pair<std::string, utils::Lines> fromFileParser( const sys_string& filename, parseContentFunc& parse ) {
    std::pair<std::string, utils::Lines> lines;
    int file = open( filename.c_str(), O_RDONLY );
    IF_RET( !file );
    utils::ScopeGuard onExit( [file] { close( file ); } );

    size_t length = utils::fileSize( file );
    IF_RET( !length );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    IF_RET( length != ( size_t )read( file, ptr, length ) );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min( length, 100ul ) ) ) );

    lines.second = parse( ptr, length );
    return lines;
}

//! POSIX API with thread local storage
std::pair<std::string, utils::Lines> fromFilePosix( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    int file = open( filename.c_str(), O_RDONLY );
    IF_RET( !file );
    utils::ScopeGuard onExit( [file] { close( file ); } );

    size_t length = utils::fileSize( file );
    IF_RET( !length );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    IF_RET( length != ( size_t )read( file, ptr, length ) );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min( length, 100ul ) ) ) );

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

//! memory mapped API with thread local storage
std::pair<std::string, utils::Lines> fromFileMmap( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    int file = open( filename.c_str(), O_RDONLY );
    IF_RET( !file );
    utils::ScopeGuard onExit( [file] { close( file ); } );

    size_t length = utils::fileSize( file );
    IF_RET( !length );

    char* map = ( char* )mmap( 0, length, PROT_READ, MAP_PRIVATE, file, 0 );
    IF_RET( map == MAP_FAILED );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( map, std::min( length, 100ul ) ) ) );

    lines.second = utils::parseContent( map, length );
    munmap( map, length ); // if used, call munmap after parsing
    return lines;
}

//! C API with thread local storage
std::pair<std::string, utils::Lines> fromFileLocal( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    FILE* file = fopen( filename.c_str(), "rb" );
    IF_RET( file == NULL );
    const utils::ScopeGuard onExit( [file] { fclose( file ); } );

    size_t length = utils::fileSize( fileno( file ) );
    IF_RET( !length );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    // read content
    IF_RET( length != fread( ptr, 1, length, file ) );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min( length, 100ul ) ) ) );

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

//! C API with string storage
std::pair<std::string, utils::Lines> fromFileString( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    FILE* file = fopen( filename.c_str(), "rb" );
    IF_RET( file == NULL );
    const utils::ScopeGuard onExit( [file] { fclose( file ); } );

    size_t length = utils::fileSize( fileno( file ) );
    IF_RET( !length );

    lines.first.resize( length );
    char* ptr = lines.first.data();

    // read content
    IF_RET( length != fread( ptr, 1, length, file ) );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min( length, 100ul ) ) ) );

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

//! CPP API with thread local storage
std::pair<std::string, utils::Lines> fromFileCPP( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    std::ifstream file( filename.c_str(), std::ios::binary | std::ios::in );

    IF_RET( !file );

    file.seekg( 0, std::ios::end );
    size_t length = ( size_t ) file.tellg();
    file.seekg( 0, std::ios::beg );

    IF_RET( !length );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    file.read( ptr, length );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min( length, 100ul ) ) ) );

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

// C API with lseek instead fstat
std::pair<std::string, utils::Lines> fromFileLSeek( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    FILE* file = fopen( filename.c_str(), "rb" );
    IF_RET( file == NULL );
    const utils::ScopeGuard onExit( [file] { fclose( file ); } );

    fseek( file, 0, SEEK_END );
    size_t length = ftell( file );
    fseek( file, 0, SEEK_SET );

    IF_RET( !length );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    IF_RET( length != fread( ptr, 1, length, file ) );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min( length, 100ul ) ) ) );

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

std::pair<std::string, utils::Lines> fromFileTwoFread( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    FILE* file = fopen( filename.c_str(), "rb" );
    IF_RET( file == NULL );
    const utils::ScopeGuard onExit( [file] { fclose( file ); } );

    size_t length = utils::fileSize( fileno( file ) );
    IF_RET( !length );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    // check first 100 bytes for binary
    if( length > 100 ) {
        IF_RET( 100 != fread( ptr, 1, 100, file ) );
        IF_RET( !utils::isTextFile( std::string_view( ptr, 100 ) ) );
        IF_RET( length - 100 != fread( ptr + 100, 1, length - 100, file ) );
    } else {
        IF_RET( length != fread( ptr, 1, length, file ) );
        IF_RET( !utils::isTextFile( std::string_view( ptr, length ) ) );
    }

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

std::pair<std::string, utils::Lines> fromFileUtils( const sys_string& filename ) {
    return fromFileParser( filename, utils::parseContent );
}

std::pair<std::string, utils::Lines> fromFileForLoop( const sys_string& filename ) {
    return fromFileParser( filename, parseContentForLoop );
}

std::pair<std::string, utils::Lines> fromFileFind( const sys_string& filename ) {
    return fromFileParser( filename, parseContentFind );
}

using fromFileFunc = std::pair<std::string, utils::Lines>( const sys_string& filename );

std::map<fromFileFunc*, const char*> names = {
    {fromFilePosix, "fromFilePosix"},
    {fromFileMmap, "fromFileMmap"},
    {fromFileLocal, "fromFileLocal"},
    {fromFileString, "fromFileString"},
    {fromFileLSeek, "fromFileLSeek"},
    {fromFileTwoFread, "fromFileTwoFread"},
    {fromFileCPP, "fromFileCPP"},
    {fromFileUtils, "fromFileUtils"},
    {fromFileForLoop, "fromFileForLoop"},
    {fromFileFind, "fromFileFind"},
    {fromFileCPP, "fromFileCPP"},
    {utils::fromFileC, "utils::fromFileC"},
};

size_t run( fromFileFunc fromFile ) {
    size_t sum = 0;
    size_t lineCount = 0;
    fs::path include = "/usr/include";

    auto tp = std::chrono::system_clock::now();

    utils::recurseDir( include.native(), [&sum, &lineCount, fromFile]( const sys_string & filename ) {
        auto lines = fromFile( filename );
        sum += lines.first.size();
        lineCount += lines.second.size();
    } );

    auto duration = std::chrono::system_clock::now() - tp;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>( duration );

    printf( "%16s : %5lu kB and %lu lines in %lld ms\n", names[fromFile], sum / 1024, lineCount, ms.count() );
    return ms.count();
}

BOOST_AUTO_TEST_SUITE( Performance )

BOOST_AUTO_TEST_CASE( Test_fromFile ) {

    size_t t2 = run( fromFileCPP );
    /*size_t tM = */run( fromFileMmap );
    /*size_t tS = */run( fromFileString );
    /*size_t tF = */run( fromFileLSeek );
    /*size_t tL = */run( fromFileLocal );
    /*size_t tO = */run( fromFileTwoFread );
    /*size_t tP = */run( fromFilePosix );
    size_t t1 = run( utils::fromFileC );
    printf( "\n" );
    BOOST_CHECK_LT( t1, t2 ); // assume FILE* is faster than std::ifstream
}

BOOST_AUTO_TEST_CASE( Test_parseContent ) {
    size_t t2 = run( fromFileForLoop );
    size_t t1 = run( fromFileUtils );
    size_t tf = run( fromFileFind );
    printf( "\n" );
    BOOST_CHECK_LT( t1, t2 );
}

BOOST_AUTO_TEST_CASE( Test_ThreadPool ) {
    boost::int_least64_t ns_asio;
    boost::int_least64_t ns_own;
    {
        boost::timer::cpu_timer stopwatch;
        stopwatch.start();
        {
            boost::asio::thread_pool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

            for( int i = 0; i < 1000; ++i ) {
                boost::asio::post( pool, [] {} );
            }

            pool.join();
        }

        stopwatch.stop();
        ns_asio = stopwatch.elapsed().wall;
    }
    {
        boost::timer::cpu_timer stopwatch;
        stopwatch.start();
        {
            ThreadPool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

            for( int i = 0; i < 1000; ++i ) {
                pool.add( [] {} );
            }
        }

        stopwatch.stop();
        ns_own = stopwatch.elapsed().wall;
    }

    printf( "own %llu us, boost %llu us\n\n", ns_own / 1000, ns_asio / 1000 );
    BOOST_CHECK_GT( ns_asio, ns_own ); // assume own tp is faster than boost::asio
}

boost::int_least64_t timed1000( const std::string& name, const std::function<void()>& func ) {
    boost::timer::cpu_timer stopwatch;
    stopwatch.start();

    for( int i = 0; i < 1000; ++i ) {
        func();
    }

    stopwatch.stop();
    boost::int_least64_t ns = stopwatch.elapsed().wall;
    std::cout << name << " : " << ns / 1000 << " us" << std::endl;
    return ns;
}

BOOST_AUTO_TEST_CASE( Test_printf ) {
    std::string text = "text123";
    FILE* file = fopen( "dump.txt", "w" );

    /*boost::int_least64_t t_write = */timed1000( "write", [file, text] {
        std::string data = "[" + text + "]\n";
        write( fileno( file ), data.c_str(), data.size() );
    } );

    fseek( file, 0, SEEK_SET );

    boost::int_least64_t t_printf = timed1000( "fprintf", [file, text] {
        fprintf( file, "%s%s]\n", "[", text.c_str() );
    } );

    fseek( file, 0, SEEK_SET );

    /*boost::int_least64_t t_fputs = */timed1000( "fputs", [file, text] {
        fputs( ( "[" + text + "]\n" ).c_str(), file );
    } );

    fseek( file, 0, SEEK_SET );

    boost::int_least64_t t_fwrite = timed1000( "fwrite", [file, text] {
        std::string data = "[" + text + "]\n";
        fwrite( data.c_str(), 1, data.size(), file );
    } );

    fseek( file, 0, SEEK_SET );

    fclose( file );
    BOOST_CHECK_LT( t_fwrite, t_printf ); // assume fwrite is faster than printf
    printf( "\n" );
}

BOOST_AUTO_TEST_CASE( Test_find ) {
    std::string text = "You can get there from here, but why on earth would you want to?";
    std::string term = "earth";
    size_t pos;
    void* ptr;

    boost::int_least64_t t_find = timed1000( "find", [&text, &term, &pos] {
        pos = text.find( term );
    } );

    boost::int_least64_t t_memmem = timed1000( "memmem", [&text, &term, &ptr] {
        ptr = memmem( text.data(), text.size(), term.data(), term.size() );
    } );

    BOOST_REQUIRE_NE( pos, std::string::npos );
    BOOST_REQUIRE_NE( ptr, nullptr );

    BOOST_CHECK_LT( t_find, t_memmem ); // assume find is faster than memmem
    printf( "\n" );
}

BOOST_AUTO_TEST_SUITE_END()

