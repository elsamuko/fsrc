#pragma once

#include "utils.hpp"

#include "boost/regex.hpp"
namespace rx = boost;

class GlobMatcher {
    public:
        GlobMatcher( std::string glob );
        bool matches( const sys_string& filename );
    private:
        rx::regex regex;
};
