#pragma once

#include "printer.hpp"

struct PipedPrinter : public Printer {
    using Print = std::function<void()>;
    std::vector<Print> prints;
    virtual void collectPrints( const sys_string& path, const std::vector<search::Match>& matches, const std::string_view& content ) override;
    virtual void printPrints() override;
    virtual ~PipedPrinter() override {}
};

void PipedPrinter::collectPrints( const sys_string& path, const std::vector<search::Match>& matches, const std::string_view& content ) {
    prints.clear();
    prints.reserve( 3 * matches.size() );

    // don't pipe colors
    Color neutral = Color::Neutral;

    // parse file for newlines until last match
    long long stop = matches.back().second - content.cbegin();
    const utils::Lines lines = utils::parseContent( content.data(), content.size(), stop );

    size_t lineNo = 0;
    size_t size = lines.size();
    size_t printed = size + 1; // init with unreachable line number

    std::vector<search::Match>::const_iterator match = matches.cbegin();
    std::vector<search::Match>::const_iterator end = matches.cend();

    size_t pos = path.rfind( '/' );

    if( pos == sys_string::npos ) { pos = path.rfind( '\\' ); }

    if( pos == sys_string::npos ) {
        pos = 0;
    } else {
        ++pos;
    }

    std::string filename( path.cbegin() + pos, path.cend() );

    for( ; match != end; ) {

        // find line for match
        while( !( match->first < lines[lineNo].cend() ) ) {
            ++lineNo;
        }

        assert( lineNo < size );

        const std::string_view& line = lines[lineNo];

        // print lineNo and code until match start
        if( printed != lineNo ) {
            printed = lineNo;

            // print file path
            prints.emplace_back( utils::printFunc( neutral, filename ) );

            // code in neutral
            prints.emplace_back( utils::printFunc( neutral, ":" ) );
            prints.emplace_back( utils::printFunc( neutral, std::string( line ) ) );
            prints.emplace_back( utils::printFunc( neutral, "\n" ) );
        }

        // search first match not in this line anymore
        while( match->first < line.cend() ) {
            match++;

            // if there are no more matches in this file, exit search for this file
            if( match == end ) {
                goto end;
            }
        }
    }

end:
    void();
}

void PipedPrinter::printPrints() {
    for( const std::function<void()>& func : prints ) { func(); }
}
