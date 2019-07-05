#include "boost/test/unit_test.hpp"
#include "stopwatch.hpp"
#include "utils.hpp"
#include "nftwwalker.hpp"
#include "ftswalker.hpp"

#include <fcntl.h>

long runDirWalkerTest( const std::string& name, const decltype( utils::recurseDir )& func ) {
    std::atomic_size_t files = 0;
    std::atomic_size_t bytes = 0;
    fs::path include = "../../../../libs/boost";

    long ns = 0;
    STOPWATCH
    START

    func( include.native(), [&bytes, &files]( const sys_string & filename ) {
        int fd = open( filename.c_str(), O_RDONLY | O_BINARY );

        if( fd != -1 ) {
            files++;
            bytes += utils::fileSize( fd );
            close( fd );
        }
    } );

    STOP( ns )

    printf( "   %20s : %zu files with %zu kB in %ld ms\n", name.c_str(), files.load(), bytes.load() / 1024, ns / 1000000 );
    return ns;
}

BOOST_AUTO_TEST_CASE( Test_DirWalker ) {
    printf( "DirWalker\n" );

    long nsFTS   = runDirWalkerTest( "withFTS", withFTS::recurseDir );
    long nsNftw  = runDirWalkerTest( "withNFTW", withNFTW::recurseDir );
    long nsUtils = runDirWalkerTest( "utils", utils::recurseDir );

    // assume, that readdir + fstat64 is still faster than nftw64
    BOOST_CHECK_LT( nsUtils, nsNftw );
}
