#pragma once

#include <list>
#include <string>
#include <fstream>
#include <algorithm>
#include <experimental/filesystem>
#include <cstdio>

#if WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace utils {
std::list<std::string> run( const std::string& command );
std::string fileHead( const std::experimental::filesystem::path& filename, const size_t count );
bool isTextFile( const std::experimental::filesystem::path& filename );
std::list<std::string> fromFile( const std::experimental::filesystem::path& filename );
}
