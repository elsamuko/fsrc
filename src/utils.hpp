#pragma once

#include <string>
#include <iostream>
#include <functional>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <Shlwapi.h>
#define popen  _popen
#define pclose _pclose
#define open   _wopen
#define fopen  _wfopen
#define close  _close
#define strcasestr StrStrIA
#define O_RDONLY _O_RDONLY
#define O_BINARY _O_BINARY
#define O_RB L"rb"
#else
#define _read read
#define O_RB "rb"
#define O_BINARY 0
#endif

#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;
using sys_string = fs::path::string_type;
namespace os = boost::system;

#define LOG( A ) std::cout << A << std::endl;

enum class Color {
    Red,
    Green,
    Blue,
    Gray,
    Neutral
};

constexpr unsigned long long int operator "" _MB( unsigned long long int in ) {
    return in * 1024 * 1024;
}

constexpr unsigned long long int operator "" _kB( unsigned long long int in ) {
    return in * 1024;
}

namespace utils {

struct ScopeGuard {
    std::function<void()> onExit;
    ScopeGuard( const std::function<void()>& onExit ) : onExit( onExit ) {}
    ~ScopeGuard() { onExit(); }
};

struct Buffer {
    char* ptr = static_cast<char*>( malloc( 1_MB ) );
    size_t size = 0;
    size_t reserved = 1_MB;
    inline char* grow( const size_t requested ) {
        if( reserved < requested ) {
            reserved = requested;
            ptr = static_cast<char*>( realloc( ptr, reserved + 1 ) );
        }

        size = requested;
        ptr[size] = '\0';
        return ptr;
    }

    ~Buffer() {
        free( ptr );
    }
};

using Lines = std::vector<std::string_view>;

struct FileView {
    size_t size = 0;
    Lines lines;
    std::string_view content;
};

//! prints text in color to stdout
void printColor( Color color, const std::string& text );

//! runs shell command
//! \returns output of command as vector
std::vector<sys_string> run( const std::string& command );

//! \returns true, if filename has no "\0\0" in the first 1000 bytes
bool isTextFile( const std::string_view& content );

#define IF_RET( A ) if( A ) { view.size = 0; return view; }

//! \returns content of filename as vector with C API
FileView fromFileP( const sys_string& filename );

#ifdef _WIN32
//! \returns content of filename as vector with WINAPI
FileView fromWinAPI( const sys_string& filename );
#endif

//! splits content at newlines
//! \returns lines as vector of string_view
Lines parseContent( const char* data, const size_t size, const long long stop );

//! \param file file descriptor
size_t fileSize( const int file );

//! \returns printf style string
template <typename ... Args>
std::string format( const char* format, Args const& ... args ) {

    size_t size = snprintf( nullptr, 0, format, args... );
    std::string text( size, '\0' );
    snprintf( text.data(), text.size() + 1, format, args... );

    return text;
}

//! \returns function, which prints format in color to stdout
inline std::function<void()> printFunc( Color color, const std::string& text ) {
    return [color, text] { printColor( color, text ); };
}

//! opens file with platforms standard program
bool openFile( const sys_string& filename );

void recurseDir( const sys_string& filename, const std::function<void( const sys_string& filename )>& callback );

}
