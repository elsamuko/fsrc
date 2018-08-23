#include "utils.hpp"

#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cstring>

#if WIN32
#include <Windows.h>
#define popen _popen
#define pclose _pclose
const std::map<Color, WORD> colors = {
    {Color::Red,     FOREGROUND_RED},
    {Color::Green,   FOREGROUND_GREEN},
    {Color::Blue,    FOREGROUND_BLUE},
};
#else
#include <dirent.h>
#include <sys/stat.h>
const std::map<Color, const char*> colors = {
    {Color::Red,     "\033[1;31m"},
    {Color::Green,   "\033[1;32m"},
    {Color::Blue,    "\033[1;34m"},
};
#endif

void utils::printColor( Color color, const std::string& text ) {
    if( color == Color::Neutral ) {
        fputs( text.c_str(), stdout );
    } else {
#if WIN32
        const static HANDLE h = ::GetStdHandle( STD_OUTPUT_HANDLE );
        const static WORD attributes = []( const HANDLE h ) {
            CONSOLE_SCREEN_BUFFER_INFO csbiInfo = {};
            ::GetConsoleScreenBufferInfo( h, &csbiInfo );
            return csbiInfo.wAttributes;
        }( h );
        ::SetConsoleTextAttribute( h, colors.at( color ) | FOREGROUND_INTENSITY );
        fputs( text.c_str(), stdout );
        ::SetConsoleTextAttribute( h, attributes );
#else
        printf( "%s%s\033[0m", colors.at( color ), text.c_str() );
#endif
    }
}

std::vector<std::string> utils::run( const std::string& command ) {
    std::string buffer( 1024, '\0' );
    std::vector<std::string> result;

    FILE* pipe = popen( command.c_str(), "r" );

    if( !pipe ) { return result; }

    while( !feof( pipe ) ) {
        if( fgets( ( char* )buffer.data(), 101, pipe ) != NULL ) {
            result.emplace_back( buffer.c_str() );
        }
    }

    for( std::string& line : result ) {
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
    bool hasDoubleZero = content.find( zerozero ) != std::string_view::npos;
    return !hasDoubleZero;
}

// splits content on newline
utils::Lines utils::parseContent( const char* data, const size_t size ) {
    Lines lines;
    lines.reserve( 128 );

    if( size == 0 ) { return lines; }

    char* c_old = ( char* )data;
    char* c_new = c_old;
    char* c_end = c_old + size;

    for( ; *c_new; ++c_new ) {
        // just skip windows line endings
        if( *c_new == '\r' ) {
            ++c_new;
        }

        if( *c_new == '\n' ) {
            lines.emplace_back( c_old, c_new - c_old );
            c_old = c_new + 1;
        }
    }

    if( c_old != c_end ) {
        lines.emplace_back( c_old, c_end - c_old );
    }

    lines.shrink_to_fit();
    return lines;
}

std::pair<std::string, utils::Lines> utils::fromFileC( const sys_string& filename ) {
    std::pair<std::string, Lines> lines;
    FILE* file = fopen( filename.c_str(), "rb" );
    IF_NOT_RET( file == nullptr );
    utils::ScopeGuard onExit( [file] { fclose( file ); } );

    size_t length = utils::fileSize( fileno( file ) );
    IF_NOT_RET( !length );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( length );

    // read content
    IF_NOT_RET( length != fread( ptr, 1, length, file ) );

    // check first 100 bytes for binary
    IF_NOT_RET( !utils::isTextFile( std::string_view( ptr, std::min( length, 100ul ) ) ) );

    lines.second = utils::parseContent( ptr, length );
    return lines;
}

#if !WIN32
void utils::recurseDir( const sys_string& filename, const std::function<void( const sys_string& filename )>& callback ) {
    DIR* dir = opendir( filename.c_str() );

    if( !dir ) { return; }

    struct dirent* dp = nullptr;

    while( ( dp = readdir( dir ) ) != NULL ) {

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

        if( data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ) {
            callback( filename + L"\\" + data.cFileName );
            continue;
        }

        if( data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY ) {
            if( !wcscmp( data.cFileName, L".." ) ) { continue; }

            if( !wcscmp( data.cFileName, L".git" ) ) { continue; }

            recurseDir( filename + L"\\" + data.cFileName, callback );
            continue;
        }
    }

    FindClose( file );
}
#endif

size_t utils::fileSize( const int file ) {
    struct stat st;

    if( 0 != fstat( file, &st ) ) { return 0; }

    return st.st_size;
}
