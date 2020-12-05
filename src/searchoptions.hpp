#pragma once

#include "utils.hpp"
#include "pipes.hpp"

#include "boost/regex.hpp"
namespace rx = boost;

struct SearchOptions {
    bool success = false;
    bool noGit = false;
    bool ignoreCase = false;
    bool isRegex = false;
    bool quiet = false;
    bool html = false;
    std::string term;
    rx::regex regex;
    fs::path path;
    sys_string prefix;
    bool piped = pipes::stdoutIsPipe();
    bool colorized = !piped;
    operator bool() const { return success; }
    static SearchOptions parseArgs( int argc, char* argv[] );
};
