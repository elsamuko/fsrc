#pragma once

#include <list>
#include <string>
#include <functional>
#include <experimental/filesystem>

enum class Color {
    Red,
    Green,
    Neutral
};

namespace utils {

//! prints text in color to stdout
void printColor( Color color, const std::string& text );

//! runs shell command
//! \returns output of command as list
std::list<std::string> run( const std::string& command );

//! gets count bytes of filename
std::string fileHead( const std::experimental::filesystem::path& filename, const size_t count );

//! \returns true, if filename has no "\0\0" in the first 1000 bytes
bool isTextFile( const std::experimental::filesystem::path& filename );

//! \returns content of filename as list
std::list<std::string> fromFile( const std::experimental::filesystem::path& filename );

//! \returns function, which prints format in color to stdout
template <typename ... Args>
std::function<void()> printFunc( Color color, const char* format, Args const& ... args ) {

    int size = snprintf( nullptr, 0, format, args... );
    std::string text( size, '\0' );
    snprintf( ( char* )text.c_str(), text.size() + 1, format, args... );

    return [color, text] { printColor( color, text ); };
}

}
