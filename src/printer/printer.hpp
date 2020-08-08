#pragma once

#include <vector>
#include <string_view>

#include "utils.hpp"
#include "types.hpp"
#include "searchoptions.hpp"

#define CUT_OFF 200

struct Printer {
    const SearchOptions& opts;
    Printer( const SearchOptions& opts ) : opts( opts ) {}
    //! collect what is printed
    virtual void collectPrints( const sys_string& path, const std::vector<search::Match>& matches, const std::string_view& content ) = 0;
    //! call print functions locked
    virtual void printPrints() = 0;
    virtual ~Printer() {}
};
