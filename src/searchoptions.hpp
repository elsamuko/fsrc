#pragma once

#include "utils.hpp"
#include "pipes.hpp"

struct SearchOptions {
    bool success = false;
    bool noGit = false;
    bool ignoreCase = false;
    bool isRegex = false;
    bool quiet = false;
    bool html = false;
    std::string term;
    fs::path path;
    std::string prefix;
    bool piped = pipes::stdoutIsPipe();
    bool colorized = !piped;
    operator bool() const { return success; }
    static SearchOptions parseArgs( int argc, char* argv[] );
};
