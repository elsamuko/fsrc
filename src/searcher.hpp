#pragma once

#include <mutex>

#if WITH_BOOST
#include "boost/regex.hpp"
namespace rx = boost;
#else
#include <regex>
namespace rx = std;
#endif

#include "utils.hpp"

struct Searcher {
    std::mutex m;
    rx::regex regex;
    std::atomic_int hits = 0;
    std::atomic_int files = 0;
    std::atomic_int filesMatched = 0;

    void search( const fs::path& path, const size_t filesize = 0 );
};
