#define BOOST_TEST_MODULE Performance

#include <boost/test/unit_test.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include "utils.hpp"

//! \returns content of filename as list with POSIX API
std::pair<std::string, utils::Lines> fromFileP( const sys_string& filename ) {
    std::pair<std::string, utils::Lines> lines;
    int file = open( filename.c_str(), O_RDONLY );
    utils::ScopeGuard onExit( [file] { if( file ) { close( file ); } } );

    if( !file ) { return lines; }

    lseek( file, 0, SEEK_END );
    size_t length = lseek( file, 0, SEEK_CUR );
    lseek( file, 0, SEEK_SET );

    if( !length ) { return lines;}

    lines.first.resize( length );

    // check first 128 bytes for binary
    if( length > 128 ) {
        read( file, ( void* )lines.first.data(), 128 );

        if( !utils::isTextFile( std::string_view( lines.first.data(), 128 ) ) ) { return lines ;}

        read( file, ( char* )lines.first.data() + 128, length - 128 );
    } else {
        read( file, ( char* )lines.first.data(), length );

        if( !utils::isTextFile( std::string_view( lines.first.data(), length ) ) ) { return lines ;}
    }

    lines.second = utils::parseContent( lines.first );
    return lines;
}

using fromFileFunc = std::pair<std::string, utils::Lines>( const sys_string& filename );

std::map<fromFileFunc*, const char*> names = {
    {fromFileP, "fromFileP"},
    {utils::fromFileC, "fromFileC"},
    {utils::fromFile, "fromFile"},
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

    printf( "%10s : %lu kB and %lu lines in %lld ms\n", names[fromFile], sum / 1024, lineCount, ms.count() );
    return ms.count();
}

BOOST_AUTO_TEST_SUITE( Performance )

BOOST_AUTO_TEST_CASE( Test_fromFile ) {
    size_t t0 = run( fromFileP );
    size_t t1 = run( utils::fromFileC );
    size_t t2 = run( utils::fromFile );
    BOOST_CHECK_LT( t1, t2 ); // assume FILE* is faster than std::ifstream
}

BOOST_AUTO_TEST_SUITE_END()

