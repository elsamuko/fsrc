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
    bool colorized = isatty( fileno( stdout ) );
    operator bool() const { return success; }
    static SearchOptions parseArgs( int argc, char* argv[] );
};

//! \returns true, if term is only alnum or underscore
inline bool isSimpleSearch( const std::string& term ) {

    for( const char& c : term ) {
        if( !std::isalnum( c ) && c != '_' ) { return false; }
    }

    return true;
}

struct Searcher {
    std::mutex m;
    bool simple = false;
    std::string term;
    rx::regex regex;
    SearchOptions opts;
    std::atomic_int hits = {0};
    std::atomic_int files = {0};
    std::atomic_int filesMatched = {0};

    Searcher( const SearchOptions& opts ) : opts( opts ) {
        term = opts.term;
        simple = isSimpleSearch( term );

        // use regex only for complex searches
        if( !simple ) {
            try {
                regex.assign( term );
            } catch( const rx::regex_error& e ) {
                LOG( "Invalid regex: " << e.what() );
            }
        }

    }

    void search( const sys_string& path );
};
