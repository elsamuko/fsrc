#include "boost/test/unit_test.hpp"
#include "stopwatch.hpp"
#include "utils.hpp"

#include <ftw.h>
#include <fcntl.h>

#ifdef __linux__
#define stat stat64
#define nftw nftw64
#endif

#define SPARE_FDS 5

static std::atomic_size_t nftwFiles = 0;
static std::atomic_size_t nftwBytes = 0;

// https://github.com/perusio/linux-programming-by-example/blob/master/book/ch08/ch08-nftw.c
size_t withNftw( const sys_string& filename, const std::function<void( const sys_string& filename )>& callback ) {

    int flags = FTW_PHYS | FTW_MOUNT | FTW_DEPTH;
    int nfds = getdtablesize() - SPARE_FDS;

    auto process = []( const char* file,
                       const struct stat * sb,
                       int flag,
                       struct FTW*  /*ftw*/
    ) -> int {
        if( flag == FTW_F ) {
            int fd = open( file, O_RDONLY | O_BINARY );

            if( fd != -1 ) {
                ++nftwFiles;
                nftwBytes += sb->st_size;
                close( fd );
            }
        }

        return 0;
    };

    if( nftw( filename.c_str(), process, nfds, flags ) != 0 ) {
        printf( "nftw returned an error\n" );
    }

    return nftwFiles;
}

BOOST_AUTO_TEST_CASE( Test_DirWalker ) {
    printf( "DirWalker\n" );

    long nsUtils = 0;
    long nsNftw = 0;
    STOPWATCH

    fs::path include = "../../../../libs/boost";

    {
        std::atomic_size_t files = 0;
        std::atomic_size_t bytes = 0;
        START

        utils::recurseDir( include.native(), [&bytes, &files]( const sys_string & filename ) {
            int fd = open( filename.c_str(), O_RDONLY | O_BINARY );

            if( fd != -1 ) {
                files++;
                bytes += utils::fileSize( fd );
                close( fd );
            }

        } );

        STOP( nsUtils )

        printf( "   utils::recurseDir : %zu files with %zu kB in %ld ms\n", files.load(), bytes.load() / 1024, nsUtils / 1000000 );
    }

    {
        START

        withNftw( include.native(), []( const sys_string& ) {} );

        STOP( nsNftw )

        printf( "            withNftw : %zu files with %zu kB in %ld ms\n", nftwFiles.load(), nftwBytes.load() / 1024, nsNftw / 1000000 );
    }

    // assume, that readdir + fstat64 is still faster than nftw64
    BOOST_CHECK_LT( nsUtils, nsNftw );
}
