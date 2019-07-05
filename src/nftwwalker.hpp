#pragma once

#include <ftw.h>
#include <fcntl.h>

#ifdef __linux__
#define stat stat64
#define nftw nftw64
#endif
#define SPARE_FDS 5

#include "utils.hpp"

namespace {

std::function<void( const sys_string& filename )> callback;

int process( const char* file, const struct stat* sb, int flag, struct FTW* ) {
    if( flag == FTW_F ) { callback( file ); }

    return 0;
}

}

namespace withNFTW {

//! \sa https://linux.die.net/man/3/nftw
//! \sa https://github.com/perusio/linux-programming-by-example/blob/master/book/ch08/ch08-nftw.c
void recurseDir( const sys_string& filename, const std::function<void( const sys_string& filename )>& callbackIn ) {

    callback = callbackIn;

    int flags = FTW_PHYS | FTW_MOUNT | FTW_DEPTH;
    int nfds = getdtablesize() - SPARE_FDS;

    if( nftw( filename.c_str(), process, nfds, flags ) != 0 ) {
        printf( "nftw returned an error\n" );
    }
}

}
