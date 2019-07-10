#pragma once

#include "printer.hpp"

struct PrettyPrinter : public Printer {
    using Print = std::function<void()>;
    std::vector<Print> prints;
    virtual void collectPrints( const sys_string& path, const std::vector<search::Match>& matches, const std::string_view& content ) override;
    virtual void printPrints() override;
    virtual ~PrettyPrinter() override {}
};

void PrettyPrinter::collectPrints( const sys_string& path, const std::vector<search::Match>& matches, const std::string_view& content ) {
    prints.clear();
    prints.reserve( 3 * matches.size() );

    // don't pipe colors
    Color cred   = opts.colorized ? Color::Red   : Color::Neutral;
    Color cblue  = opts.colorized ? Color::Blue  : Color::Neutral;
    Color cgreen = opts.colorized ? Color::Green : Color::Neutral;

    // print file path
#ifdef _WIN32
    prints.emplace_back( utils::printFunc( cgreen, std::string( path.cbegin(), path.cend() ) ) );
#else
    prints.emplace_back( utils::printFunc( cgreen, path ) );
#endif

    // parse file for newlines until last match
    long long stop = matches.back().second - content.cbegin();
    const utils::Lines lines = utils::parseContent( content.data(), content.size(), stop );

    size_t lineNo = 0;
    size_t size = lines.size();
    size_t printed = size + 1; // init with unreachable line number

    std::vector<search::Match>::const_iterator match = matches.cbegin();
    std::vector<search::Match>::const_iterator end = matches.cend();

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

            // line in blue
            std::string number = utils::format( "\nL%4i : ", lineNo + 1 );
            prints.emplace_back( utils::printFunc( cblue, number ) );

            // code in neutral
            if( line.cbegin() < match->first ) {
                prints.emplace_back( utils::printFunc( Color::Neutral, std::string( line.cbegin(), match->first ) ) );
            }
        }


        // print match in red
        prints.emplace_back( utils::printFunc( cred, std::string( match->first, match->second ) ) );

        // set from to end of match
        search::Iter from = match->second;

        // if there are no more matches in this file, print rest of line in neutral
        // and exit search for this file
        if( std::next( match ) == end ) {
            if( from < line.cend() ) {
                prints.emplace_back( utils::printFunc( Color::Neutral, std::string( from, line.cend() ) ) );
            }

            goto end;
        }

        ++match;

        // if next match is within this line, print code in neutral until next match
        if( match->first < line.cend() ) {
            prints.emplace_back( utils::printFunc( Color::Neutral, std::string( from, match->first ) ) );
        }
        // else print code in neutral until end
        else {
            prints.emplace_back( utils::printFunc( Color::Neutral, std::string( from, line.cend() ) ) );
        }
    }

end:
    // print newlines
    prints.emplace_back( utils::printFunc( Color::Neutral, "\n\n" ) );
}

void PrettyPrinter::printPrints() {
    for( const std::function<void()>& func : prints ) { func(); }
}
