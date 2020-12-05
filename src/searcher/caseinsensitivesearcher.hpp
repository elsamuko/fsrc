#pragma once

#include "searcher.hpp"
#include "utils.hpp"
#include "types.hpp"

struct CaseInsensitiveSearcher : public Searcher {
    CaseInsensitiveSearcher( const SearchOptions& opts ) : Searcher( opts ) {}
    virtual std::vector<search::Match> search( const std::string_view& content ) override;
    virtual ~CaseInsensitiveSearcher() {}
};

std::vector<search::Match> CaseInsensitiveSearcher::search( const std::string_view& content ) {
    std::vector<search::Match> matches;

    search::Iter pos = content.cbegin();
    const char* start = content.data();
    const char* ptr = start;

    while( ( ptr = strcasestr( ptr, opts.term.data() ) ) ) {
        search::Iter from = pos + ( ptr - start );
        search::Iter to = from + opts.term.size();
        matches.emplace_back( from, to );
        ptr += opts.term.size();
    }

    return matches;
}
