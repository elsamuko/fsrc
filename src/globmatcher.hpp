#pragma once

#include "utils.hpp"

#include "boost/regex.hpp"
namespace rx = boost;

class GlobMatcher {
    public:
        explicit GlobMatcher( std::string glob );
        bool matches( const sys_string& filename );
        operator bool() const { return !regex.empty(); }
    private:
        rx::regex regex;
};
