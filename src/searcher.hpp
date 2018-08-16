#pragma once

#include <mutex>
#include <atomic>

#include "boost/regex.hpp"
namespace rx = boost;

#if WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#include "utils.hpp"

struct SearchOptions {
    bool success = false;
    bool noGit = false;
    std::string term;
    fs::path path;
    bool colored = isatty( fileno( stdout ) );
    operator bool() const { return success; }
    static SearchOptions parseArgs( int argc, char* argv[] );
};

struct Searcher {
    std::mutex m;
    rx::regex regex;
    SearchOptions opts;
    std::atomic_int hits = {0};
    std::atomic_int files = {0};
    std::atomic_int filesMatched = {0};

    Searcher( const SearchOptions& opts ) : opts( opts ) {
        try {
            regex = opts.term;
        } catch( const rx::regex_error& e ) {
            LOG( "Invalid regex: " << e.what() );
        }
    }

    void search( const fs::path& path, const size_t filesize = 0 );
};
