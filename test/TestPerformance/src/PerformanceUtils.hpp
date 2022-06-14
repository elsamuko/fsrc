#pragma once

#include <iostream>
#include <functional>
#include <fcntl.h>

#include "utils.hpp"
#include "stopwatch.hpp"
#include "threadpool.hpp"

using fromFileFunc = utils::FileView( const sys_string& filename );
using Result = std::pair<long, std::string>;

inline void printSorted( std::vector<Result>& results ) {
    std::sort( results.begin(), results.end(), []( const Result & a, const Result & b ) {
        return a.first < b.first;
    } );

    for( const Result& result : results ) {
        printf( "%s", result.second.c_str() );
    }
}

inline Result timed1000( const std::string& name, const std::function<void()>& func, const std::function<void()>& cleanup = std::function<void()>() ) {
    long ns = 0;
    STOPWATCH
    START

    for( int i = 0; i < 1000; ++i ) {
        func();
    }

    STOP( ns );

    // call cleanup function
    if( cleanup ) { cleanup(); }

    return { ns, utils::format( "%17s : %6ld us\n", name.c_str(), ns / 1000 ) };
}

inline Result runDirWalkerTest( const std::string& name, decltype( utils::recurseDir )& func ) {
    std::atomic_size_t files = 0;
    std::atomic_size_t bytes = 0;
    fs::path include = "../../../../libs/boost/include/";

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

    return {ns, utils::format( "   %20s : %zu files with %zu kB in %ld ms\n", name.c_str(), files.load(), bytes.load() / 1024, ns / 1000000 )};
}

inline Result run( fromFileFunc fromFile, const char* name ) {
    size_t sum = 0;
    size_t lineCount = 0;
    size_t files = 0;

    //! \note Windows is slow in I/O
#if BOOST_OS_WINDOWS
    fs::path include = "../../../../libs/boost/include/boost/asio/";
#else
    fs::path include = "../../../../libs/boost";
#endif

    long ns = 0;
    STOPWATCH
    START

    utils::recurseDir( include.native(), [&sum, &lineCount, &files, fromFile]( const sys_string & filename ) {
        auto view = fromFile( filename );
        files++;
        sum += view.size;
        lineCount += view.lines.size();
    } );

    STOP( ns );

    return { ns / 1000000, utils::format( "%17s : %zu files, %5zu kB and %zu lines in %ld ms\n",
                                          name, files, sum / 1024, lineCount, ns / 1000000 ) };
}
