#pragma once

#include <list>
#include <string>
#include <functional>

#if WITH_BOOST
#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;
namespace os = boost::system;
#else
#include <system_error>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
namespace os = std;
#endif


enum class Color {
    Red,
    Green,
    Blue,
    Neutral
};

namespace utils {

//! prints text in color to stdout
void printColor( Color color, const std::string& text );

//! runs shell command
//! \returns output of command as list
std::list<std::string> run( const std::string& command );

//! gets count bytes of filename
std::string fileHead( const fs::path& filename, const size_t count );

//! \returns true, if filename has no "\0\0" in the first 1000 bytes
bool isTextFile( const fs::path& filename );

//! \returns content of filename as list
std::pair<std::string, std::list<std::string_view>> fromFile( const fs::path& filename );

//! \returns function, which prints format in color to stdout
template <typename ... Args>
std::function<void()> printFunc( Color color, const char* format, Args const& ... args ) {

    int size = snprintf( nullptr, 0, format, args... );
    std::string text( size, '\0' );
    snprintf( ( char* )text.c_str(), text.size() + 1, format, args... );

    return [color, text] { printColor( color, text ); };
}

#if !WIN32
void recurseDirUnix( const std::string& filename, const std::function<void( const std::string& filename )> callback );
#endif

}