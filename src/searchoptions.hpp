#pragma once

#include "utils.hpp"
#include "pipes.hpp"

#include "boost/regex.hpp"
namespace rx = boost;

struct SearchOptions {
    bool success = false;
    bool noGit = false;         // do not use git ls-files
    bool ignoreCase = false;    // case insensitive search
    bool isRegex = false;       // regex search
    bool quiet = false;         // print only status
    bool html = false;          // open results as html page
    bool onlyFiles = false;     // print only filenames
    bool noURI = false;         // print w/out file://
    bool piped = pipes::stdoutIsPipe(); // grep-compatible piped output
    bool colorized = !piped; // show colors
    std::string term;
    std::string glob;
    rx::regex regex;
    fs::path path;
    sys_string pathPrefix;
    operator bool() const { return success; }
    static SearchOptions parseArgs( int argc, char* argv[] );
};
