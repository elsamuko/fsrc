#include "utils.hpp"

#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cstring>

#include <boost/algorithm/string.hpp>

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

// binary files have usually zero padding
bool utils::isTextFile( const std::string_view& content ) {
    //! \note https://en.wikipedia.org/wiki/List_of_file_signatures

    // PDF -> binary
    if( boost::algorithm::starts_with( content, "%PDF" ) ) { return false; }

    // PostScript -> binary
    if( boost::algorithm::starts_with( content, "%!PS" ) ) { return false; }

    bool hasDoubleZero = content.find( std::string( { 0, 0 } ) ) != std::string::npos;
    return !hasDoubleZero;
}

// splits content on newline
std::list<std::string_view> utils::parseContent( const std::string& content ) {
    std::list<std::string_view> lines;
    const char* data = content.data();
    int pos = 0;
    size_t length = content.size();

    for( size_t i = 0; i < length; ++i ) {
        // just skip windows line endings
        if( content[i] == '\r' ) {
            ++i;
        }

        // cut at unix line endings
        if( content[i] == '\n' ) {
            lines.emplace_back( data + pos, i - pos );
            pos = i + 1;
        }
    }

    return lines;
}

std::pair<std::string, std::list<std::string_view>> utils::fromFileC( const sys_string& filename ) {
    std::pair<std::string, std::list<std::string_view>> lines;
    FILE* file = fopen( filename.c_str(), "rb" );

    if( file == NULL ) { return lines; }

    fseek( file, 0, SEEK_END );
    size_t length = ftell( file );
    fseek( file, 0, SEEK_SET );

    if( !length ) { return lines;}

    lines.first.resize( length );

    // check first 100 bytes for binary
    if( length > 100 ) {
        fread( ( void* )lines.first.data(), 1, 100, file );

        if( !utils::isTextFile( std::string_view( lines.first.data(), 100 ) ) ) { return lines ;}

        fread( ( char* )lines.first.data() + 100, 1, length - 100, file );
    } else {
        fread( ( char* )lines.first.data(), 1, length, file );

        if( !utils::isTextFile( std::string_view( lines.first.data(), length ) ) ) { return lines ;}
    }

    fclose( file );
    lines.second = utils::parseContent( lines.first );
    return lines;
}

std::pair<std::string, std::list<std::string_view>> utils::fromFile( const sys_string& filename ) {
    std::pair<std::string, std::list<std::string_view>> lines;
    std::ifstream file( filename.c_str(), std::ios::binary | std::ios::in );

    if( !file ) { return lines;}

    file.seekg( 0, std::ios::end );
    size_t length = ( size_t ) file.tellg();
    file.seekg( 0, std::ios::beg );

    if( !length ) { return lines;}

    lines.first.resize( length );

    // check first 100 bytes for binary
    if( length > 100 ) {
        file.read( ( char* ) lines.first.data(), 100 );

        if( !utils::isTextFile( std::string_view( lines.first.data(), 100 ) ) ) { return lines ;}

        file.read( ( char* ) lines.first.data() + 100, length - 100 );
    } else {
        file.read( ( char* ) lines.first.data(), length );

        if( !utils::isTextFile( std::string_view( lines.first.data(), length ) ) ) { return lines ;}
    }

    lines.second = utils::parseContent( lines.first );
    return lines;
}

#if !WIN32
void utils::recurseDir( const sys_string& filename, const std::function<void( const sys_string& filename )>& callback ) {
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

        if( !wcscmp( data.cFileName, L".." ) ) { continue; }

        if( !wcscmp( data.cFileName, L".git" ) ) { continue; }

        if( data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ) {
            callback( filename + L"\\" + data.cFileName );
            continue;
        }

        if( data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY ) {
            recurseDir( filename + L"\\" + data.cFileName, callback );
            continue;
        }
    }

    FindClose( file );
}
#endif
