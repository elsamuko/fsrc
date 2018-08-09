#pragma once

#include <map>
#include <list>
#include <string>
#include <iostream>
#include <functional>
#include <experimental/filesystem>

#if WIN32
#define popen _popen
#define pclose _pclose
#endif

enum class Color {
    Red,
    Green,
    Neutral
};

const std::map<Color, std::string> colors = {
    {Color::Red,     "\033[1;31m"},
    {Color::Green,   "\033[1;32m"},
};

namespace utils {
std::list<std::string> run( const std::string& command );
std::string fileHead( const std::experimental::filesystem::path& filename, const size_t count );
bool isTextFile( const std::experimental::filesystem::path& filename );
std::list<std::string> fromFile( const std::experimental::filesystem::path& filename );
template <typename ... Args>
std::function<void()> printFunc( Color color, const std::string& format, Args const& ... args ) {

    int size = snprintf( nullptr, 0, format.c_str(), args... );
    std::string text( size, '\0' );
    snprintf( ( char* )text.c_str(), text.size() + 1, format.c_str(), args... );

    return [text, color] {
#if WIN32
        std::cout << text << std::flush;
#else

        if( color == Color::Neutral ) {
            std::cout << text << std::flush;
        } else {
            std::cout << colors.at( color ) << text << "\033[0m" << std::flush;
        }

#endif

    };
}
}
