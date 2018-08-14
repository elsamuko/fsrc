#pragma once

#include <mutex>
#include <atomic>

#include "boost/regex.hpp"
namespace rx = boost;

#if WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#include "utils.hpp"

struct Searcher {
    std::mutex m;
    rx::regex regex;
    bool colored = isatty( fileno( stdout ) );
    std::atomic_int hits = 0;
    std::atomic_int files = 0;
    std::atomic_int filesMatched = 0;

    void search( const fs::path& path, const size_t filesize = 0 );
};
