#pragma once

#include <mutex>
#include <atomic>

#include "boost/regex.hpp"
#include "boost/algorithm/searching/boyer_moore_horspool.hpp"

namespace rx = boost;
#include "utils.hpp"
#include "searchoptions.hpp"

struct Searcher {
    std::mutex m;
    std::string term;
    rx::regex regex;
    SearchOptions opts;
    std::atomic_size_t count = {0};
    std::atomic_size_t files = {0};
    std::atomic_size_t filesMatched = {0};
    boost::algorithm::boyer_moore_horspool<std::string::iterator>* bmh;

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

        if( !opts.isRegex && !opts.ignoreCase ) {
            bmh = new boost::algorithm::boyer_moore_horspool( term.begin(), term.end() );
        }
    }

    void search( const sys_string& path );
};
