#pragma once

#include <list>
#include <string>
#include <iostream>
#include <functional>

#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;
using sys_string = fs::path::string_type;
namespace os = boost::system;

#define LOG( A ) std::cout << A << std::endl;

enum class Color {
    Red,
    Green,
    Blue,
    Neutral
};

#if WIN32
namespace std {
struct string_view {
    const char* data = nullptr;
    size_t size = 0;
    string_view( const char* data = nullptr, size_t size = 0 ) :
        data( data ), size( size ) {}
    bool empty() const { return size == 0 ; }
    const char& front() const { return data[0]; }
    const char& back() const { return data[size - 1]; }
};
}
#endif

namespace utils {

struct ScopeGuard {
    std::function<void()> onExit;
    ScopeGuard( const std::function<void()>& onExit ) : onExit( onExit ) {}
    ~ScopeGuard() { onExit(); }
};

using Lines = std::vector<std::string_view>;

//! prints text in color to stdout
void printColor( Color color, const std::string& text );

//! runs shell command
//! \returns output of command as list
std::list<std::string> run( const std::string& command );

//! \returns true, if filename has no "\0\0" in the first 1000 bytes
bool isTextFile( const std::string_view& content );

//! \returns content of filename as list with C API
std::pair<std::string, Lines> fromFileC( const sys_string& filename );

//! \returns content of filename as list with C++ API
std::pair<std::string, Lines> fromFile( const sys_string& filename );

//! splits content at newlines
//! \returns lines as list of string_view
Lines parseContent( const std::string& content );

//! \returns function, which prints format in color to stdout
template <typename ... Args>
std::function<void()> printFunc( Color color, const char* format, Args const& ... args ) {

    int size = snprintf( nullptr, 0, format, args... );
    std::string text( size, '\0' );
    snprintf( ( char* )text.c_str(), text.size() + 1, format, args... );

    return [color, text] { printColor( color, text ); };
}

void recurseDir( const sys_string& filename, const std::function<void( const sys_string& filename )>& callback );

}
