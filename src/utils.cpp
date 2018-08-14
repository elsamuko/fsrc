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
const std::map<Color, std::string> colors = {
    {Color::Red,     "\033[1;31m"},
    {Color::Green,   "\033[1;32m"},
    {Color::Blue,    "\033[1;34m"},
};
#endif

void utils::printColor( Color color, const std::string& text ) {
    if( color == Color::Neutral ) {
        std::cout << text << std::flush;
    } else {
#if WIN32
        HANDLE h = ::GetStdHandle( STD_OUTPUT_HANDLE );
        CONSOLE_SCREEN_BUFFER_INFO csbiInfo = {};
        ::GetConsoleScreenBufferInfo( h, &csbiInfo );
        ::SetConsoleTextAttribute( h, colors.at( color ) | FOREGROUND_INTENSITY );
        std::cout << text << std::flush;
        ::SetConsoleTextAttribute( h, csbiInfo.wAttributes );
#else
        std::cout << colors.at( color ) << text << "\033[0m" << std::flush;
#endif
    }
}

std::list<std::string> utils::run( const std::string& command ) {
    std::string buffer( 1024, '\0' );
    std::list<std::string> result;

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

std::string utils::fileHead( const fs::path& filename, const size_t count ) {
    std::ifstream file( filename.c_str(), std::ios::binary | std::ios::in );
    file.seekg( 0, std::ios::end );
    size_t length = ( size_t ) file.tellg();
    file.seekg( 0, std::ios::beg );

    // read count bytes, or length, if file is too small
    if( length > count ) { length = count; }

    const std::string content( length, '\0' );
    file.read( ( char* ) content.data(), length );
    return content;
}

// binary files have usually zero padding
bool utils::isTextFile( const fs::path& filename ) {
    std::string head = fileHead( filename, 1000 );
    bool hasDoubleZero = head.find( std::string( { 0, 0 } ) ) != std::string::npos;
    return !hasDoubleZero;
}

std::pair<std::string, std::list<std::string_view>> utils::fromFile( const fs::path& filename, const size_t filesize ) {
    std::pair<std::string, std::list<std::string_view>> lines;
    std::ifstream file( filename.c_str(), std::ios::binary | std::ios::in );

    if( !file ) { return lines;}

    size_t length = filesize;

    if( !filesize ) {
        file.seekg( 0, std::ios::end );
        length = ( size_t ) file.tellg();
        file.seekg( 0, std::ios::beg );
    }

    if( !length ) { return lines;}

    lines.first.resize( length );
    file.read( ( char* ) lines.first.data(), length );
    const char* data = lines.first.data();

    int pos = 0;

    for( size_t i = 0; i < length; ++i ) {
        // just skip windows line endings
        if( lines.first[i] == '\r' ) {
            ++i;
        }

        // cut at unix line endings
        if( lines.first[i] == '\n' ) {
            lines.second.emplace_back( data + pos, i - pos );
            pos = i + 1;
        }
    }

    return lines;
}

#if !WIN32
void utils::recurseDirUnix( const std::string& filename, const std::function<void( const std::string& filename )>& callback ) {
    DIR* dir = opendir( filename.c_str() );
    struct dirent* dp = nullptr;

    while( ( dp = readdir( dir ) ) != NULL ) {
        if( !strcmp( dp->d_name, "." ) ) { continue; }

        if( !strcmp( dp->d_name, ".." ) ) { continue; }

        if( !strcmp( dp->d_name, ".git" ) ) { continue; }

        if( dp->d_type == DT_REG ) {
            callback( filename + "/" + dp->d_name );
            continue;
        }

        if( dp->d_type == DT_DIR ) {
            utils::recurseDirUnix( filename + "/" + dp->d_name, callback );
            continue;
        }

        // if( dp->d_type == DT_LNK ) { continue; }
    }

    closedir( dir );
}
#else
void utils::recurseDirWin( const std::wstring& filename, const std::function<void ( const std::wstring& filename, const size_t filesize )>& callback ) {
    WIN32_FIND_DATAW data = {};

    std::wstring withGlob = filename + L"\\*";
    HANDLE file = FindFirstFileExW( withGlob.c_str(), FindExInfoBasic, &data, FindExSearchNameMatch, nullptr, 0 );

    if( !file ) { return; }

    while( FindNextFileW( file, &data ) ) {

        if( !wcscmp( data.cFileName, L".." ) ) { continue; }

        if( !wcscmp( data.cFileName, L".git" ) ) { continue; }

        if( data.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE ) {
            callback( filename + L"\\" + data.cFileName, data.nFileSizeLow );
            continue;
        }

        if( data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY ) {
            recurseDirWin( filename + L"\\" + data.cFileName, callback );
            continue;
        }
    }

    FindClose( file );
}
#endif
