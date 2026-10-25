#pragma once

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "utils.hpp"

namespace withOpenAt {

namespace {
//! \param fd opened directory, is closed afterwards
//! \param path path of fd, is restored afterwards
inline void recurseDirFd( const int fd, sys_string& path, const std::function<void( const sys_string& filename )>& callback ) {
    DIR* dir = fdopendir( fd );

    if( !dir ) { close( fd ); return; }

    // add slash only, if there is none
    if( path.back() != '/' ) { path.push_back( '/' ); }

    const size_t size = path.size();

    struct dirent* dp = nullptr;

    while( ( dp = readdir( dir ) ) != nullptr ) {

        if( dp->d_type == DT_REG ) {
            path.resize( size );
            path += dp->d_name;
            callback( path );
            continue;
        }

        if( dp->d_type == DT_DIR ) {
            if( !strcmp( dp->d_name, "." ) ) { continue; }

            if( !strcmp( dp->d_name, ".." ) ) { continue; }

            if( !strcmp( dp->d_name, ".git" ) ) { continue; }

            if( !strcmp( dp->d_name, ".svn" ) ) { continue; }

            if( !strcmp( dp->d_name, ".hg" ) ) { continue; }

            // open subdir relative to this dir, so the kernel doesn't resolve the full path again
            const int sub = openat( dirfd( dir ), dp->d_name, O_RDONLY | O_DIRECTORY | O_CLOEXEC );

            if( sub != -1 ) {
                path.resize( size );
                path += dp->d_name;
                recurseDirFd( sub, path, callback );
            }

            continue;
        }
    }

    path.resize( size );
    closedir( dir );
}
}

//! like utils::recurseDir, but with openat/fdopendir and one reused path buffer
//! \note opening files relative to the dir fd was slower in fsrc, see review.md
inline void recurseDir( const sys_string& filename, const std::function<void( const sys_string& filename )>& callback ) {
    const int fd = open( filename.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC );

    if( fd == -1 ) { return; }

    sys_string path = filename;
    recurseDirFd( fd, path, callback );
}

}
