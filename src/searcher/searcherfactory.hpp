#pragma once

#include "regexsearcher.hpp"
#include "casesensitivesearcher.hpp"
#include "caseinsensitivesearcher.hpp"
#include "searchoptions.hpp"

namespace searcherfactory {

std::function<Searcher*()> searcherFunc( SearchOptions& opts ) {

    if( opts.isRegex ) {
        return [&opts] {
            rx::regex::flag_type flags = rx::regex::normal;

            if( opts.ignoreCase ) { flags ^= rx::regex::icase; }

            try {
                opts.regex.assign( opts.term, flags );
            } catch( const rx::regex_error& e ) {
                LOG( "Invalid regex: " << e.what() );
                exit( EXIT_FAILURE );
            }

            RegexSearcher* searcher = new RegexSearcher( opts );
            return searcher;
        };
    }

    if( opts.ignoreCase ) {
        return [&opts] {
            CaseInsensitiveSearcher* searcher = new CaseInsensitiveSearcher( opts );
            return searcher;
        };

    }

    return [&opts] {
        CaseSensitiveSearcher* searcher = new CaseSensitiveSearcher( opts );
        return searcher;
    };
}

}
