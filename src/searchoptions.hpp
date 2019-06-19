#pragma once

#include "utils.hpp"

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

struct SearchOptions {
    bool success = false;
    bool noGit = false;
    bool ignoreCase = false;
    bool isRegex = false;
    std::string term;
    fs::path path;
    bool colorized = isatty( fileno( stdout ) ) != 0;
    operator bool() const { return success; }
    static SearchOptions parseArgs( int argc, char* argv[] );
};
