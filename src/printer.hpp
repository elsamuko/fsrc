#pragma once

#include <vector>
#include <string_view>

#include "utils.hpp"
#include "types.hpp"

struct PrintOptions {
    bool colorized = true;
};

struct Printer {
    PrintOptions opts;
    //! collect what is printed
    virtual void collectPrints( const sys_string& path, const std::vector<search::Match>& matches, const std::string_view& content ) = 0;
    //! call print functions locked
    virtual void printPrints() = 0;
    virtual ~Printer() {}
};
