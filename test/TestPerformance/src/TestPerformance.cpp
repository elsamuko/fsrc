#define BOOST_TEST_MODULE Performance

#include <boost/test/unit_test.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "utils.hpp"
#include "threadpool.hpp"

#include "boost/asio.hpp"
using boost::asio::post;

//! POSIX API with thread local storage
std::pair<std::string, utils::Lines> fromFileP( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    int file = open( filename.c_str(), O_RDONLY );
    utils::ScopeGuard onExit( [file] { if( file ) { close( file ); } } );

    if( !file ) { return lines; }

    lseek( file, 0, SEEK_END );
    size_t length = lseek( file, 0, SEEK_CUR );
    lseek( file, 0, SEEK_SET );

    if( !length ) { return lines;}

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    // check first 128 bytes for binary
    if( length > 128 ) {
        read( file, ptr, 128 );

        if( !utils::isTextFile( std::string_view( ptr, 128 ) ) ) { return lines ;}

        read( file, ptr + 128, length - 128 );
    } else {
        read( file, ptr, length );

        if( !utils::isTextFile( std::string_view( ptr, length ) ) ) { return lines ;}
    }

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

//! memory mapped API with thread local storage
std::pair<std::string, utils::Lines> fromFileM( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    int file = open( filename.c_str(), O_RDONLY );
    IF_NOT_RET( !file );
    utils::ScopeGuard onExit( [file] { close( file ); } );

    size_t length = utils::fileSize( file );
    IF_NOT_RET( !length );

    char* map = ( char* )mmap( 0, length, PROT_READ, MAP_PRIVATE, file, 0 );
    IF_NOT_RET( map == MAP_FAILED );

    // check first 128 bytes for binary
    IF_NOT_RET( !utils::isTextFile( std::string_view( map, std::min( length, 100ul ) ) ) );

    lines.second = utils::parseContent( map, length );
    munmap( map, length ); // if used, call munmap after parsing
    return lines;
}

//! C API with thread local storage
std::pair<std::string, utils::Lines> fromFileL( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    FILE* file = fopen( filename.c_str(), "rb" );
    const utils::ScopeGuard onExit( [file] { if( file ) { fclose( file ); } } );

    if( file == NULL ) { return lines; }

    fseek( file, 0, SEEK_END );
    size_t length = ftell( file );
    fseek( file, 0, SEEK_SET );

    if( !length ) { return lines;}

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    // check first 100 bytes for binary
    if( length > 100 ) {
        if( 100 != fread( ptr, 1, 100, file ) ) { return lines; }

        if( !utils::isTextFile( std::string_view( ptr, 100 ) ) ) { return lines ;}

        if( length - 100 != fread( ptr + 100, 1, length - 100, file ) ) { return lines; }
    } else {
        if( length != fread( ptr, 1, length, file ) ) { return lines; }

        if( !utils::isTextFile( std::string_view( ptr, length ) ) ) { return lines ;}
    }

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

//! C API with string storage
std::pair<std::string, utils::Lines> fromFileS( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    FILE* file = fopen( filename.c_str(), "rb" );
    const utils::ScopeGuard onExit( [file] { if( file ) { fclose( file ); } } );

    if( file == NULL ) { return lines; }

    fseek( file, 0, SEEK_END );
    size_t length = ftell( file );
    fseek( file, 0, SEEK_SET );

    if( !length ) { return lines;}

    lines.first.resize( length );
    char* ptr = lines.first.data();

    // check first 100 bytes for binary
    if( length > 100 ) {
        if( 100 != fread( ptr, 1, 100, file ) ) { return lines; }

        if( !utils::isTextFile( std::string_view( ptr, 100 ) ) ) { return lines ;}

        if( length - 100 != fread( ptr + 100, 1, length - 100, file ) ) { return lines; }
    } else {
        if( length != fread( ptr, 1, length, file ) ) { return lines; }

        if( !utils::isTextFile( std::string_view( ptr, length ) ) ) { return lines ;}
    }

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

//! CPP API with thread local storage
std::pair<std::string, utils::Lines> fromFileCPP( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    std::ifstream file( filename.c_str(), std::ios::binary | std::ios::in );

    if( !file ) { return lines;}

    file.seekg( 0, std::ios::end );
    size_t length = ( size_t ) file.tellg();
    file.seekg( 0, std::ios::beg );

    if( !length ) { return lines;}

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    // check first 100 bytes for binary
    if( length > 100 ) {
        file.read( ptr, 100 );

        if( !utils::isTextFile( std::string_view( ptr, 100 ) ) ) { return lines ;}

        file.read( ptr + 100, length - 100 );
    } else {
        file.read( ptr, length );

        if( !utils::isTextFile( std::string_view( ptr, length ) ) ) { return lines ;}
    }

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

// C API with lseek instead fstat
std::pair<std::string, utils::Lines> fromFileF( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    FILE* file = fopen( filename.c_str(), "rb" );
    utils::ScopeGuard onExit( [file] { if( file ) { fclose( file ); } } );

    if( file == NULL ) { return lines; }

    fseek( file, 0, SEEK_END );
    size_t length = ftell( file );
    fseek( file, 0, SEEK_SET );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    // check first 100 bytes for binary
    if( length > 100 ) {
        if( 100 != fread( ptr, 1, 100, file ) ) { return lines; }

        if( !utils::isTextFile( std::string_view( ptr, 100 ) ) ) { return lines ;}

        if( length - 100 != fread( ptr + 100, 1, length - 100, file ) ) { return lines; }
    } else {
        if( length != fread( ptr, 1, length, file ) ) { return lines; }

        if( !utils::isTextFile( std::string_view( ptr, length ) ) ) { return lines ;}
    }

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

std::pair<std::string, utils::Lines> fromFileTwoFread( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    FILE* file = fopen( filename.c_str(), "rb" );
    utils::ScopeGuard onExit( [file] { if( file ) { fclose( file ); } } );

    if( file == NULL ) { return lines; }

    struct stat st;

    if( 0 != fstat( fileno( file ), &st ) ) { return lines; }

    size_t length = st.st_size;

    if( !length ) { return lines;}

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    // check first 100 bytes for binary
    if( length > 100 ) {
        if( 100 != fread( ptr, 1, 100, file ) ) { return lines; }

        if( !utils::isTextFile( std::string_view( ptr, 100 ) ) ) { return lines ;}

        if( length - 100 != fread( ptr + 100, 1, length - 100, file ) ) { return lines; }
    } else {
        if( length != fread( ptr, 1, length, file ) ) { return lines; }

        if( !utils::isTextFile( std::string_view( ptr, length ) ) ) { return lines ;}
    }

    // check first 100 bytes for binary
    if( !utils::isTextFile( std::string_view( ptr, 100 ) ) ) { return lines ;}

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

using fromFileFunc = std::pair<std::string, utils::Lines>( const sys_string& filename );

std::map<fromFileFunc*, const char*> names = {
    {fromFileP, "fromFileP"},
    {fromFileM, "fromFileM"},
    {fromFileL, "fromFileL"},
    {fromFileS, "fromFileS"},
    {fromFileF, "fromFileF"},
    {fromFileTwoFread, "fromFileTwoFread"},
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
    /*size_t tP = */run( fromFileP );
    /*size_t tM = */run( fromFileM );
    /*size_t tL = */run( fromFileL );
    /*size_t tS = */run( fromFileS );
    /*size_t tF = */run( fromFileF );
    /*size_t tO = */run( fromFileTwoFread );
    size_t t1 = run( utils::fromFileC );
    size_t t2 = run( fromFileCPP );
    BOOST_CHECK_LT( t1, t2 ); // assume FILE* is faster than std::ifstream
}

BOOST_AUTO_TEST_CASE( Test_ThreadPool ) {
    size_t ms_asio;
    size_t ms_own;
    {
        auto tp = std::chrono::system_clock::now();
        {
            boost::asio::thread_pool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

            for( int i = 0; i < 1000; ++i ) {
                boost::asio::post( pool, [] {} );
            }

            pool.join();
        }

        auto duration = std::chrono::system_clock::now() - tp;
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>( duration );
        ms_asio = ms.count();
    }
    {
        auto tp = std::chrono::system_clock::now();
        {
            ThreadPool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

            for( int i = 0; i < 1000; ++i ) {
                pool.add( [] {} );
            }
        }

        auto duration = std::chrono::system_clock::now() - tp;
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>( duration );
        ms_own = ms.count();
    }

    printf( "own %lu ms, boost %lu ms\n", ms_own, ms_asio );
    BOOST_CHECK_GT( ms_asio, ms_own ); // assume own tp is faster than boost::asio
}

BOOST_AUTO_TEST_SUITE_END()

