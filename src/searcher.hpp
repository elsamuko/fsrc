#pragma once

#include <mutex>
#include <atomic>

#include "boost/regex.hpp"

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

    using Iter = std::string_view::const_iterator;
    using Hit = std::pair<Iter, Iter>;
    using Print = std::function<void()>;

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
                exit( EXIT_FAILURE );
            }
        }
    }

    ~Searcher() {}

    void search( const sys_string& path );

    //! search with strcasestr
    std::vector<Hit> caseInsensitiveSearch( const std::string_view& content );
    //! search with strstr or string_view::find
    std::vector<Hit> caseSensitiveSearch( const std::string_view& content );
    //! search with boost::regex
    std::vector<Hit> regexSearch( const std::string_view& content );
    //! collect what is printed
    std::vector<Print> collectPrints( const sys_string& path, const std::vector<Hit>& hits, const std::string_view& content );
    //! call print functions locked
    void printPrints( const std::vector<Print>& prints );
};
