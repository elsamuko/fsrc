#include "utils.hpp"

#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>

#if WIN32
#include "Windows.h"
#define popen _popen
#define pclose _pclose
const std::map<Color, WORD> colors = {
    {Color::Red,     FOREGROUND_RED},
    {Color::Green,   FOREGROUND_GREEN},
    {Color::Blue,    FOREGROUND_BLUE},
};
#else
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
    std::string buffer( 100, '\0' );
    std::list<std::string> result;

    FILE* pipe = popen( command.c_str(), "r" );

    if( !pipe ) { return result; }

    while( !feof( pipe ) ) {
        if( fgets( ( char* )buffer.data(), 101, pipe ) != NULL ) {
            result.push_back( buffer.c_str() );
        }
    }

    for( std::string& line : result ) {
        line.pop_back(); // remove newline
    }

    pclose( pipe );
    return result;
}

std::string utils::fileHead( const std::experimental::filesystem::path& filename, const size_t count ) {
    std::ifstream file( filename.c_str(), std::ios::binary );
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
bool utils::isTextFile( const std::experimental::filesystem::path& filename ) {
    std::string head = fileHead( filename, 1000 );
    bool hasDoubleZero = head.find( std::string( { 0, 0 } ) ) != std::string::npos;
    return !hasDoubleZero;
}

std::list<std::string> utils::fromFile( const std::experimental::filesystem::path& filename ) {
    std::list<std::string> lines;
    std::ifstream file( filename.c_str(), std::ios::binary );

    if( !file ) { return lines;}

    file.seekg( 0, std::ios::end );
    size_t length = ( size_t ) file.tellg();
    file.seekg( 0, std::ios::beg );

    const std::string content( length, '\0' );
    file.read( ( char* ) content.data(), length );
    const char* data = content.data();

    int pos = 0;

    for( size_t i = 0; i < length; ++i ) {
        // just skip windows line endings
        if( content[i] == '\r' ) {
            ++i;
        }

        // cut at unix line endings
        if( content[i] == '\n' ) {
            lines.emplace_back( data + pos, data + i );
            pos = i + 1;
        }
    }

    return lines;
}
