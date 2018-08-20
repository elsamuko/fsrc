#define BOOST_TEST_MODULE Performance

#include <boost/test/unit_test.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include "utils.hpp"

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

    boost::iostreams::mapped_file_source file( filename );

    if( !file.is_open() ) { return lines; }

    size_t length = file.size();

    if( !length ) { return lines;}

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    // check first 128 bytes for binary
    if( length > 128 ) {
        if( !utils::isTextFile( std::string_view( file.data(), 128 ) ) ) { return lines ;}
    } else {
        if( !utils::isTextFile( std::string_view( file.data(), length ) ) ) { return lines ;}
    }

    memcpy( ptr, file.data(), length );

    lines.second = utils::parseContent( ptr, length );
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

using fromFileFunc = std::pair<std::string, utils::Lines>( const sys_string& filename );

std::map<fromFileFunc*, const char*> names = {
    {fromFileP, "fromFileP"},
    {fromFileM, "fromFileM"},
    {fromFileL, "fromFileL"},
    {fromFileS, "fromFileS"},
    {fromFileCPP, "fromFileCPP"},
    {utils::fromFileC, "fromFileC"},
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

    printf( "%12s : %5lu kB and %lu lines in %lld ms\n", names[fromFile], sum / 1024, lineCount, ms.count() );
    return ms.count();
}

BOOST_AUTO_TEST_SUITE( Performance )

BOOST_AUTO_TEST_CASE( Test_fromFile ) {
    /*size_t tP = */run( fromFileP );
    /*size_t tM = */run( fromFileM );
    /*size_t tL = */run( fromFileL );
    /*size_t tS = */run( fromFileS );
    size_t t1 = run( utils::fromFileC );
    size_t t2 = run( fromFileCPP );
    BOOST_CHECK_LT( t1, t2 ); // assume FILE* is faster than std::ifstream
}

BOOST_AUTO_TEST_SUITE_END()

