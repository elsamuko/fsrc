#pragma once

#include <vector>
#include <string_view>

#include "types.hpp"
#include "searchoptions.hpp"

struct Searcher {
    const SearchOptions& opts;
    Searcher( const SearchOptions& opts ) : opts( opts ) {}
    virtual std::vector<search::Match> search( const std::string_view& content ) = 0;
    virtual ~Searcher() {}
};
