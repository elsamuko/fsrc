#include "utils.hpp"

#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fcntl.h>

#include "pipes.hpp"
#include "stdstr.hpp"

#ifdef _WIN32
#include <Windows.h>

const std::map<Color, WORD> winColors = {
    {Color::Red,     FOREGROUND_RED},
    {Color::Green,   FOREGROUND_GREEN},
    {Color::Blue,    FOREGROUND_BLUE | FOREGROUND_GREEN},
    {Color::Gray,    FOREGROUND_INTENSITY},
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

#endif

const std::map<Color, std::string> bashColors = {
    {Color::Red,     "\033[1;31m"},
    {Color::Green,   "\033[1;32m"},
    {Color::Blue,    "\033[1;34m"},
    {Color::Gray,    "\033[38;5;245m"},
    {Color::Reset,   "\033[0m"},
};

void utils::printColor( Color color, const std::string& text ) {
    if( color == Color::Neutral ) {
        fwrite( text.c_str(), 1, text.size(), stdout );
    } else {
#ifdef _WIN32

        if( pipes::stdoutIsPipedPty() ) {
            std::string data = bashColors.at( color ) + text + bashColors.at( Color::Reset );
            fwrite( data.c_str(), 1, data.size(), stdout );
        } else {

            const HANDLE h = ::GetStdHandle( STD_OUTPUT_HANDLE );
            const static WORD attributes = []( const HANDLE h ) {
                CONSOLE_SCREEN_BUFFER_INFO csbiInfo = {};
                ::GetConsoleScreenBufferInfo( h, &csbiInfo );
                return csbiInfo.wAttributes;
            }( h );
            const static WORD background = attributes & ( 0x00F0 );
            ::SetConsoleTextAttribute( h, background | winColors.at( color ) | FOREGROUND_INTENSITY );
            fwrite( text.c_str(), 1, text.size(), stdout );
            ::SetConsoleTextAttribute( h, attributes );
        }

#else
        std::string data = bashColors.at( color ) + text + bashColors.at( Color::Reset );
        fwrite( data.c_str(), 1, data.size(), stdout );
#endif
    }
}

// git ls-files -zco --exclude-standard | tr '\0' '\n'
void utils::gitLsFiles( const fs::path& path, const std::function<void( const sys_string& filename )>& callback ) {

    fs::current_path( path );

#ifdef _WIN32
    std::string nullDevice = "NUL";
#else
    std::string nullDevice = "/dev/null";
#endif

    // -c Show cached files in the output (default)
    // -o Show other (i.e. untracked) files in the output
    // -z \0 line termination on output and do not quote filenames
    const std::string command = "git ls-files -coz --exclude-standard 2> " + nullDevice;

    const size_t size = 1_kB;
    sys_string buffer( size, '\0' );
    const sys_string::value_type* first = &buffer.front();
    const sys_string::value_type* last  = &buffer.back();
    sys_string rest;

    FILE* pipe = popen( command.c_str(), "r" );

    if( !pipe ) { return; }

    while( !feof( pipe ) ) {
#ifdef _WIN32

        if( fgetws( buffer.data(), 1_kB, pipe ) != nullptr ) {
#else

        if( fgets( buffer.data(), 1_kB, pipe ) != nullptr ) {
#endif
            const sys_string::value_type* from = first;

            // search first path
            const sys_string::value_type* to = std::char_traits<sys_string::value_type>::find( from, size, '\0' );

            if( !to ) { goto end; }

            // and prepend rest to it
            callback( rest + sys_string( from, to ) );

            from = to + 1;

            // search for git's single null terminators
            while( ( to = std::char_traits<sys_string::value_type>::find( from, last - from, '\0' ) ) ) {
                // two nulls -> no more output
                if( from == to ) { goto end; }

                callback( sys_string( from, to ) );
                from = to + 1;
            }

            // keep rest for next fgets round
            rest = sys_string( from, last );

            // and clear buffer so we don't read old data
            memset( buffer.data(), '\0', buffer.size() * sizeof( sys_string::value_type ) );
        }
    }

end:
    pclose( pipe );
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

    const bool hasDoubleZero = fromStd::strstr( content.data(), content.size(), "\0\0", 2 ) != nullptr;
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

    while( ( c_new = std::char_traits<char>::find( c_old, c_end - c_old, '\n' ) ) ) {
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

    // read first 4 kB
    size_t offset = std::min<size_t>( view.size, 4_kB );
    size_t bytes = _read( file, ptr, offset );
    IF_RET( offset != bytes );

    // check first 300 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min<size_t>( offset, 300ul ) ) ) );

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

    // read first 4 kB
    size_t offset = std::min<size_t>( view.size, 4_kB );
    BOOL ok = ::ReadFile( file,
                          ptr,
                          offset,
                          &read,
                          nullptr );
    IF_RET( !ok );

    // check first 300 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min<size_t>( offset, 300ul ) ) ) );

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

    // add slash only, if there is none
    const char* slash = filename.back() == '/' ? "" : "/";

    struct dirent* dp = nullptr;

    while( ( dp = readdir( dir ) ) != nullptr ) {

        if( dp->d_type == DT_REG ) {
            callback( filename + slash + dp->d_name );
            continue;
        }

        if( dp->d_type == DT_DIR ) {
            if( !strcmp( dp->d_name, "." ) ) { continue; }

            if( !strcmp( dp->d_name, ".." ) ) { continue; }

            if( !strcmp( dp->d_name, ".git" ) ) { continue; }

            if( !strcmp( dp->d_name, ".svn" ) ) { continue; }

            if( !strcmp( dp->d_name, ".hg" ) ) { continue; }

            utils::recurseDir( filename + slash + dp->d_name, callback );
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

            if( !wcscmp( data.cFileName, L".svn" ) ) { continue; }

            if( !wcscmp( data.cFileName, L".hg" ) ) { continue; }

            recurseDir( filename + data.cFileName + L"\\", callback );
            continue;
        }

        if( data.dwFileAttributes & ( FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL ) ) {
            callback( filename + data.cFileName );
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

#if BOOST_OS_LINUX
bool utils::openFile( const sys_string& filename ) {
    std::string command = "xdg-open " + filename;
    return 0 == system( command.c_str() );
}
#elif BOOST_OS_WINDOWS
//! \note Must run on main thread!
bool utils::openFile( const sys_string& filename ) {
    HINSTANCE rv = ::ShellExecuteW( nullptr, // HWND   hwnd
                                    L"open", // LPCWSTR lpOperation,
                                    filename.c_str(),
                                    nullptr, // LPCWSTR lpParameters,
                                    nullptr, // LPCWSTR lpDirectory,
                                    SW_SHOW ); // INT    nShowCmd

    return ( int )rv > 32;
}
#else
// mac's impl is in macutils.mm
#endif

#if BOOST_OS_WINDOWS
sys_string utils::absolutePath( const sys_string& filename ) {
    sys_string rv( 1024, '\0' );
    DWORD size = ::GetFullPathNameW( filename.c_str(), rv.size(), rv.data(), nullptr );
    rv.resize( size );
    return rv;
}
#else
sys_string utils::absolutePath( const sys_string& filename ) {
    sys_string rv( PATH_MAX, '\0' );
    ::realpath( filename.c_str(), rv.data() );
    rv.resize( strlen( rv.data() ) );
    return rv;
}
#endif
