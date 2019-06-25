#include <chrono>

#include "boost/test/unit_test.hpp"

#include "utils.hpp"

#include <ftw.h>
#include <fcntl.h>

#define SPARE_FDS 5

static size_t nftwCount = 0;

// https://github.com/perusio/linux-programming-by-example/blob/master/book/ch08/ch08-nftw.c
size_t withNftw( const sys_string& filename, const std::function<void( const sys_string& filename )>& callback ) {

    int flags = FTW_PHYS | FTW_MOUNT | FTW_DEPTH;
    int nfds = getdtablesize() - SPARE_FDS;

    auto process = []( const char* file,
                       const struct stat64 * sb,
                       int flag,
                       struct FTW * ftw
    ) -> int {
        if( flag == FTW_F ) {
            ++nftwCount;
        }

        return 0;
    };

    if( nftw64( filename.c_str(), process, nfds, flags ) != 0 ) {
        printf( "nftw returned an error\n" );
    }

    return nftwCount;
}

BOOST_AUTO_TEST_CASE( Test_DirWalker ) {
    printf( "DirWalker\n" );

    std::chrono::milliseconds msNftw;
    std::chrono::milliseconds msUtils;

#if !BOOST_OS_WINDOWS
    fs::path include = "/usr/include";
#else
    fs::path include = std::string( getenv( "VS140COMNTOOLS" ) ) + "..\\..\\VC\\include";
#endif

    {
        size_t files = 0;
        int fd = open( "/usr/include/zlib.h", O_RDONLY | O_BINARY );

        auto tp = std::chrono::system_clock::now();

        utils::recurseDir( include.native(), [&files, &fd]( const sys_string& ) {
            // call fstat on zlib.h to be fair to nftw64, which provides a stat64 for the current file
            struct stat64 st {};
            fstat64( fd, &st );
            files++;
        } );

        auto duration = std::chrono::system_clock::now() - tp;
        msUtils = std::chrono::duration_cast<std::chrono::milliseconds>( duration );

        printf( "   utils::recurseDir : %zu files in %ld ms\n", files, msUtils.count() );
    }

    {
        auto tp = std::chrono::system_clock::now();

        size_t files = withNftw( include.native(), [&files]( const sys_string& ) {} );

        auto duration = std::chrono::system_clock::now() - tp;
        msNftw = std::chrono::duration_cast<std::chrono::milliseconds>( duration );

        printf( "            withNftw : %zu files in %ld ms\n", files, msNftw.count() );
    }

    // assume, that readdir + fstat64 is still faster than nftw64
    BOOST_CHECK_LT( msUtils.count(), msNftw.count() );
}
