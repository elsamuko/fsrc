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
    bool ignoreCase = false;
    bool isRegex = false;
    std::string term;
    fs::path path;
    bool colorized = isatty( fileno( stdout ) ) != 0;
    operator bool() const { return success; }
    static SearchOptions parseArgs( int argc, char* argv[] );
};

struct Searcher {
    std::mutex m;
    std::string term;
    rx::regex regex;
    SearchOptions opts;
    std::atomic_int hits = {0};
    std::atomic_int files = {0};
    std::atomic_int filesMatched = {0};

    Searcher( const SearchOptions& opts ) : opts( opts ) {
        term = opts.term;

        // use regex only for complex searches
        if( opts.isRegex ) {
            rx::regex::flag_type flags = rx::regex::normal;

            if( opts.ignoreCase ) { flags ^= boost::regex::icase; }

            try {
                regex.assign( term, flags );
            } catch( const rx::regex_error& e ) {
                LOG( "Invalid regex: " << e.what() );
            }
        }

    }

    void search( const sys_string& path );
};
