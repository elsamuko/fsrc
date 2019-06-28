#include "utils.hpp"

#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cstring>

#include <fcntl.h>

#ifdef _WIN32
#include <Windows.h>

const std::map<Color, WORD> colors = {
    {Color::Red,     FOREGROUND_RED},
    {Color::Green,   FOREGROUND_GREEN},
    {Color::Blue,    FOREGROUND_BLUE | FOREGROUND_GREEN},
};
#else
#include <dirent.h>
#include <sys/stat.h>

#ifdef __linux__
#define fwrite fwrite_unlocked
#define open open64
#define readdir readdir64
#define dirent dirent64
#define stat stat64
#define fstat fstat64
#endif

const std::map<Color, std::string> colors = {
    {Color::Red,     "\033[1;31m"},
    {Color::Green,   "\033[1;32m"},
    {Color::Blue,    "\033[1;34m"},
};
#endif

void utils::printColor( Color color, const std::string& text ) {
    if( color == Color::Neutral ) {
        fwrite( text.c_str(), 1, text.size(), stdout );
    } else {
#ifdef _WIN32
        const HANDLE h = ::GetStdHandle( STD_OUTPUT_HANDLE );
        const static WORD attributes = []( const HANDLE h ) {
            CONSOLE_SCREEN_BUFFER_INFO csbiInfo = {};
            ::GetConsoleScreenBufferInfo( h, &csbiInfo );
            return csbiInfo.wAttributes;
        }( h );
        const static WORD background = attributes & ( 0x00F0 );
        ::SetConsoleTextAttribute( h, background | colors.at( color ) | FOREGROUND_INTENSITY );
        fwrite( text.c_str(), 1, text.size(), stdout );
        ::SetConsoleTextAttribute( h, attributes );
#else
        std::string data = colors.at( color ) + text + "\033[0m";
        fwrite( data.c_str(), 1, data.size(), stdout );
#endif
    }
}

std::vector<sys_string> utils::run( const std::string& command ) {
    sys_string buffer( 1_kB, '\0' );
    std::vector<sys_string> result;

    FILE* pipe = popen( command.c_str(), "r" );

    if( !pipe ) { return result; }

    while( !feof( pipe ) ) {
#ifdef _WIN32

        if( fgetws( buffer.data(), 101, pipe ) != nullptr ) {
#else

        if( fgets( buffer.data(), 101, pipe ) != nullptr ) {
#endif
            result.emplace_back( buffer.c_str() );
        }
    }

    for( sys_string& line : result ) {
        line.pop_back(); // remove newline
    }

    pclose( pipe );
    return result;
}

// binary files have usually zero padding
bool utils::isTextFile( const std::string_view& content ) {
    //! \note https://en.wikipedia.org/wiki/List_of_file_signatures

    if( content.size() >= 4 ) {
        // PDF -> binary
        if( 0 == memcmp( content.data(), "%PDF", 4 ) ) { return false; }

        // PostScript -> binary
        if( 0 == memcmp( content.data(), "%!PS", 4 ) ) { return false; }
    }

    static std::string_view zerozero( "\0\0", 2 );
    bool hasDoubleZero = content.find( zerozero ) != std::string::npos;
    return !hasDoubleZero;
}

// splits content on newline
utils::Lines utils::parseContent( const char* data, const size_t size, const long long stop ) {
    Lines lines;
    lines.reserve( 128 );

    if( size == 0 ) { return lines; }

    const char* c_old = data;
    const char* c_new = c_old;
    const char* c_end = c_old + size;

    while( ( c_new = static_cast<const char*>( memchr( c_old, '\n', c_end - c_old ) ) ) ) {
        lines.emplace_back( c_old, c_new - c_old );
        c_old = c_new + 1;

        // only parse newlines until stop bytes
        if( ( c_old - data ) > stop ) { c_old = c_end; break; }
    }

    if( c_old != c_end ) {
        lines.emplace_back( c_old, c_end - c_old );
    }

    lines.shrink_to_fit();
    return lines;
}

utils::FileView utils::fromFileP( const sys_string& filename ) {
    FileView view;
    int file = open( filename.c_str(), O_RDONLY | O_BINARY );
    IF_RET( file == -1 );
    utils::ScopeGuard onExit( [file] { close( file ); } );

    view.size = utils::fileSize( file );
    IF_RET( !view.size );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( view.size );

    // read first 16 kB
    size_t offset = std::min<size_t>( view.size, 4_kB );
    size_t bytes = _read( file, ptr, offset );
    IF_RET( offset != bytes );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min<size_t>( offset, 100ul ) ) ) );

    // read rest
    if( view.size > offset ) {
        size_t newSize = view.size - offset;
        size_t bytes2 = _read( file, ptr + offset, newSize );
        IF_RET( newSize != bytes2 );
    }

    view.content = std::string_view( ptr, view.size );
    return view;
}

#ifdef _WIN32
utils::FileView utils::fromWinAPI( const sys_string& filename ) {
    utils::FileView view;
    HANDLE file = ::CreateFileW( filename.c_str(),      // file to open
                                 GENERIC_READ,          // open for reading
                                 FILE_SHARE_READ,       // share for reading
                                 nullptr,               // default security
                                 OPEN_EXISTING,         // existing file only
                                 FILE_FLAG_SEQUENTIAL_SCAN |
                                 FILE_ATTRIBUTE_NORMAL, // normal file
                                 nullptr );
    IF_RET( file == INVALID_HANDLE_VALUE );
    utils::ScopeGuard onExit( [file] { ::CloseHandle( file ); } );

    view.size = ::GetFileSize( file, nullptr );
    IF_RET( !view.size );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( view.size );
    DWORD read = 0;

    // read first 16 kB
    size_t offset = std::min<size_t>( view.size, 4_kB );
    BOOL ok = ::ReadFile( file,
                          ptr,
                          offset,
                          &read,
                          nullptr );
    IF_RET( !ok );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min<size_t>( offset, 100ul ) ) ) );

    // read rest
    if( view.size > offset ) {
        BOOL ok2 = ::ReadFile( file,
                               ptr + offset,
                               view.size - offset,
                               &read,
                               nullptr );
        IF_RET( !ok2 );
    }

    view.content = std::string_view( ptr, view.size );
    return view;
}
#endif

#ifndef _WIN32
void utils::recurseDir( const sys_string& filename, const std::function<void( const sys_string& filename )>& callback ) {
    DIR* dir = opendir( filename.c_str() );

    if( !dir ) { return; }

    struct dirent* dp = nullptr;

    while( ( dp = readdir( dir ) ) != nullptr ) {

        if( dp->d_type == DT_REG ) {
            callback( filename + "/" + dp->d_name );
            continue;
        }

        if( dp->d_type == DT_DIR ) {
            if( !strcmp( dp->d_name, "." ) ) { continue; }

            if( !strcmp( dp->d_name, ".." ) ) { continue; }

            if( !strcmp( dp->d_name, ".git" ) ) { continue; }

            utils::recurseDir( filename + "/" + dp->d_name, callback );
            continue;
        }

        // if( dp->d_type == DT_LNK ) { continue; }
    }

    closedir( dir );
}
#else
void utils::recurseDir( const sys_string& filename, const std::function<void ( const sys_string& filename )>& callback ) {
    WIN32_FIND_DATAW data = {};

    std::wstring withGlob = filename + L"\\*";
    HANDLE file = FindFirstFileExW( withGlob.c_str(), FindExInfoBasic, &data, FindExSearchNameMatch, nullptr, 0 );

    if( !file ) { return; }

    while( FindNextFileW( file, &data ) ) {

        if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
            if( !wcscmp( data.cFileName, L".." ) ) { continue; }

            if( !wcscmp( data.cFileName, L".git" ) ) { continue; }

            recurseDir( filename + L"\\" + data.cFileName, callback );
            continue;
        }

        if( data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ) {
            callback( filename + L"\\" + data.cFileName );
            continue;
        }
    }

    FindClose( file );
}
#endif

size_t utils::fileSize( const int file ) {
    struct stat st {};

    if( 0 != fstat( file, &st ) ) { return 0; }

    return st.st_size;
}
