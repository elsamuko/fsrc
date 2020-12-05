#pragma once

#include "searcher.hpp"

#include "mischasan.hpp"
#include "ssefind.hpp"
#include "stdstr.hpp"

#define FIND_MISCHASAN    0
#define FIND_SSE_OWN      1
#define FIND_TRAITS       2
#define FIND_STRSTR       3

struct CaseSensitiveSearcher : public Searcher {
    CaseSensitiveSearcher( const SearchOptions& opts ) : Searcher( opts ) {}
    virtual std::vector<search::Match> search( const std::string_view& content ) override;
    virtual ~CaseSensitiveSearcher() {}
};

std::vector<search::Match> CaseSensitiveSearcher::search( const std::string_view& content ) {
#if FIND_ALGO == FIND_SSE_OWN
    return sse::find( content, opts.term );
#else

    std::vector<search::Match> matches;

    search::Iter pos = content.cbegin();
    const char* start = content.data();
    const char* ptr = start;
    const char* end = start + content.size();

#if FIND_ALGO == FIND_MISCHASAN

    while( ( ptr = mischasan::scanstrN( ptr, end - ptr, opts.term.data(), opts.term.size() ) ) )
#elif FIND_ALGO == FIND_MISCHASAN

    while( ( ptr = fromStd::strstr( ptr, end - ptr, opts.term.data(), opts.term.size() ) ) )
#elif FIND_ALGO == FIND_STRSTR

    while( ( ptr = strstr( ptr, opts.term.data() ) ) )
#endif

    {
        search::Iter from = pos + ( ptr - start );
        search::Iter to = from + opts.term.size();
        matches.emplace_back( from, to );
        ptr += opts.term.size();
    }

    return matches;
#endif
}
