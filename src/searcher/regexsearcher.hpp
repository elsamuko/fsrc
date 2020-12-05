#pragma once

#include "searcher.hpp"
#include "utils.hpp"
#include "types.hpp"

struct RegexSearcher : public Searcher {
    RegexSearcher( const SearchOptions& opts ) : Searcher( opts ) {}
    virtual std::vector<search::Match> search( const std::string_view& content ) override;
    virtual ~RegexSearcher() {}
};

std::vector<search::Match> RegexSearcher::search( const std::string_view& content ) {
    std::vector<search::Match> matches;

    // https://www.boost.org/doc/libs/1_70_0/libs/regex/doc/html/boost_regex/ref/match_flag_type.html
    rx::regex_constants::match_flags flags = rx::regex_constants::match_not_dot_newline;

    auto begin = rx::cregex_iterator( &content.front(), 1 + &content.back(), opts.regex, flags );
    auto end   = rx::cregex_iterator();

    for( rx::cregex_iterator match = begin; match != end; ++match ) {
        search::Iter from = content.cbegin() + match->position();
        search::Iter to = from + match->length();
        matches.emplace_back( from, to );
    }

    return matches;
}
