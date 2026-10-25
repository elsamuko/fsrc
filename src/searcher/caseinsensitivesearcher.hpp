#pragma once

#include "searcher.hpp"
#include "utils.hpp"
#include "types.hpp"
#include "mischasan.hpp"

struct CaseInsensitiveSearcher : public Searcher {
    std::string lowerTerm;

    CaseInsensitiveSearcher( const SearchOptions& opts ) : Searcher( opts ), lowerTerm( opts.term ) {
        for( char& c : lowerTerm ) { c = lowerAscii( c ); }
    }
    virtual std::vector<search::Match> search( const std::string_view& content ) override;
    virtual ~CaseInsensitiveSearcher() {}
};

std::vector<search::Match> CaseInsensitiveSearcher::search( const std::string_view& content ) {
    std::vector<search::Match> matches;

    search::Iter pos = content.cbegin();
    const char* start = content.data();
    const char* ptr = start;
    const char* end = start + content.size();

    while( ( ptr = mischasan::scancasestrN( ptr, end - ptr, lowerTerm.data(), lowerTerm.size() ) ) ) {
        search::Iter from = pos + ( ptr - start );
        search::Iter to = from + lowerTerm.size();
        matches.emplace_back( from, to );
        ptr += lowerTerm.size();
    }

    return matches;
}
