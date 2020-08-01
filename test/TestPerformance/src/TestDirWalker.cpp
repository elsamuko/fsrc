#ifndef __APPLE__
#include <experimental/filesystem>
#endif
#include <system_error>

#include "boost/test/unit_test.hpp"
#include "boost/filesystem.hpp"

#include "threadpool.hpp"
#include "stopwatch.hpp"
#include "utils.hpp"
#include "nftwwalker.hpp"
#include "ftswalker.hpp"

#include "PerformanceUtils.hpp"

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

#ifndef __APPLE__
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
#endif


BOOST_AUTO_TEST_CASE( Test_DirWalker ) {
    printf( "DirWalker\n" );

    std::vector<Result> results = {
        runDirWalkerTest( "withFTS", withFTS::recurseDir ),
        runDirWalkerTest( "withNFTW", withNFTW::recurseDir ),
        runDirWalkerTest( "utils", utils::recurseDir ),
        runDirWalkerTest( "withBoost", withBoost::recurseDir ),
#ifndef __APPLE__
        runDirWalkerTest( "withStd", withStd::recurseDir ),
#endif
    };

    printSorted( results );
    printf( "\n" );
}
