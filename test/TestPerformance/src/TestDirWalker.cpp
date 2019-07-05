#include <experimental/filesystem>
#include <system_error>
#include <fcntl.h>

#include "boost/test/unit_test.hpp"
#include "boost/filesystem.hpp"

#include "threadpool.hpp"
#include "stopwatch.hpp"
#include "utils.hpp"
#include "nftwwalker.hpp"
#include "ftswalker.hpp"

namespace withBoost {
void recurseDir( const sys_string& filename, const std::function<void( const sys_string& filename )>& callback ) {
    os::error_code ec;
    auto start = fs::recursive_directory_iterator( filename, ec );
    auto end   = fs::recursive_directory_iterator();

    if( ec ) { return; }

    while( start != end ) {
        fs::file_status status = start.status();

        if( fs::is_regular_file( status ) ) {
            fs::path path = start->path();
            callback( path.native() );
        }

        start++;
    }
}
}

namespace withStd {
namespace stdfs = std::experimental::filesystem;
void recurseDir( const sys_string& filename, const std::function<void( const sys_string& filename )>& callback ) {
    std::error_code ec;
    auto start = stdfs::recursive_directory_iterator( filename, ec );
    auto end   = stdfs::recursive_directory_iterator();

    if( ec ) { return; }

    while( start != end ) {

        stdfs::path path = start->path();
        stdfs::file_status status = stdfs::status( path );

        if( stdfs::is_regular_file( status ) ) {
            callback( path.native() );
        }

        start++;
    }
}
}

long runDirWalkerTest( const std::string& name, const decltype( utils::recurseDir )& func ) {
    std::atomic_size_t files = 0;
    std::atomic_size_t bytes = 0;
    fs::path include = "../../../../libs/boost/include";

    long ns = 0;
    {
        POOL;
        STOPWATCH
        START

        func( include.native(), [&pool, &bytes, &files]( const sys_string & filename ) {
            pool.add( [&bytes, &files, filename] {
                int fd = open( filename.c_str(), O_RDONLY | O_BINARY );

                if( fd != -1 ) {
                    files++;
                    bytes += utils::fileSize( fd );
                    close( fd );
                }
            } );
        } );

        STOP( ns )
    }

    printf( "   %20s : %zu files with %zu kB in %ld ms\n", name.c_str(), files.load(), bytes.load() / 1024, ns / 1000000 );
    return ns;
}

BOOST_AUTO_TEST_CASE( Test_DirWalker ) {
    printf( "DirWalker\n" );

    long nsFTS   = runDirWalkerTest( "withFTS", withFTS::recurseDir );
    long nsNftw  = runDirWalkerTest( "withNFTW", withNFTW::recurseDir );
    long nsUtils = runDirWalkerTest( "utils", utils::recurseDir );
    long nsBoost = runDirWalkerTest( "withBoost", withBoost::recurseDir );
    long nsStdFS = runDirWalkerTest( "withStd", withStd::recurseDir );

    // assume, that readdir is faster than nftw
    BOOST_CHECK_LT( nsUtils, nsNftw );
}
