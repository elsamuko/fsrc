#pragma once

#include <mutex>
#include <sstream>

#include "boost/filesystem.hpp"

#include "printer.hpp"
#include "utils.hpp"
#include "exitqueue.hpp"

namespace  HTML {
const std::string css = R"css(<style>
    body {
        color:#eee;
        background:#333;
    }
    h1 {
        font-size: 1.3em;
    }
    .result {
        white-space: pre;
        font-family: monospace;
    }
    .file {
        text-decoration: none;
        color:greenyellow;
    }
    .line {
        color:cyan;
    }
    .match {
        color:red;
    }
    .gray {
        color:gray;
    }
    .code {
        color:#eee;
    }
</style>
)css";

std::string encode( const std::string text ) {
    std::stringstream encoded;

    for( const char c : text ) {
        switch( c ) {
            case '"': encoded << "&quot;"; break;

            case '&': encoded << "&amp;"; break;

            case '<': encoded << "&lt;"; break;

            case '>': encoded << "&gt;"; break;

            case '\r': break;

            default: encoded << c;
        }
    }

    return encoded.str();
}
}

struct HtmlPrinter : public Printer {
    static std::once_flag oneHeader;
    static fs::path html;

    std::stringstream result;
    virtual void collectPrints( const sys_string& path, const std::vector<search::Match>& matches, const std::string_view& content ) override;
    virtual void printPrints() override;
    HtmlPrinter( const SearchOptions& opts ) : Printer( opts ) {
        std::call_once( oneHeader, [this] {

            fs::path dir = fs::temp_directory_path( ) / "fsrc";
            fs::create_directories( dir );
            html = dir / "results.html";
            fs::ofstream of( html, std::ios::out | std::ios::binary );

            if( of ) {
                of << "<!DOCTYPE html>\n"
                   << "<html>\n\n"
                   << "<head>\n"
                   << "<title>fsrc \"" << HTML::encode( this->opts.term ) << "\"</title>\n"
                   << HTML::css
                   << "</head>\n\n"
                   << "<body>\n"
                   << "<h1>fsrc results for \"" << HTML::encode( this->opts.term )
                   << "\" in " << HTML::encode( fs::absolute( this->opts.path ).string() )
                   << "</h1>\n\n";
            }

            of.close();

            Color gray = this->opts.colorized ? Color::Gray : Color::Neutral;

            ExitQueue::add( [gray] {
                fs::ofstream of( html, std::ios::out | std::ios::binary | std::ios::app );

                if( of ) {
                    of << "</body>\n\n"
                       << "</html>\n";
                }

                of.close();
                utils::printColor( gray, utils::format( "\nOpening \"%s\".\n", html.string().c_str() ) );
                utils::openFile( html.native() );
            } );
        } );
    }
    inline void ellipsis() {
        result << "<span class=\"gray\">...</span>";
    }
};

std::once_flag HtmlPrinter::oneHeader;
fs::path HtmlPrinter::html;

void HtmlPrinter::collectPrints( const sys_string& path, const std::vector<search::Match>& matches, const std::string_view& content ) {
    result.str( std::string() );
    fs::path file( path );

    // open result
    result << "<div class=\"result\">\n";

    // print file path
    result << "<a class=\"file\" href=\"file://"
           << HTML::encode( fs::absolute( file ).string() )
           << "\" download>"
           << HTML::encode( file.string() ) <<
           "</a>\n";

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
            std::string number = utils::format( "L%4i : ", lineNo + 1 );
            result << "<span class=\"line\">" << HTML::encode( number ) << "</span>";

            // code in neutral
            if( line.cbegin() < match->first ) {
                // elide left if line is too long
                if( match->first - line.cbegin() > CUT_OFF ) {
                    this->ellipsis();
                    result << "<span class=\"code\">" << HTML::encode( std::string( match->first - CUT_OFF, match->first ) ) << "</span>";
                } else {
                    result << "<span class=\"code\">" << HTML::encode( std::string( line.cbegin(), match->first ) ) << "</span>";
                }
            }
        }

        // print match in red
        result << "<span class=\"match\">" << HTML::encode( std::string( match->first, match->second ) ) << "</span>";

        // set from to end of match
        search::Iter from = match->second;

        // if there are no more matches in this file, print rest of line in neutral
        // and exit search for this file
        if( std::next( match ) == end ) {
            if( from < line.cend() ) {
                // elide right if line is too long
                if( line.cend() - from > CUT_OFF ) {
                    result << "<span class=\"code\">" << HTML::encode( std::string( from, from + CUT_OFF ) ) << "</span>";
                    this->ellipsis();
                } else {
                    result << "<span class=\"code\">" << HTML::encode( std::string( from, line.cend() ) ) << "</span>\n";
                }

            }

            goto end;
        }

        ++match;

        // if next match is within this line, print code in neutral until next match
        if( match->first < line.cend() ) {
            if( match->first - from > CUT_OFF ) {
                // elide middle if line is too long
                result << "<span class=\"code\">" << HTML::encode( std::string( from, from + CUT_OFF / 2 ) ) << "</span>";
                this->ellipsis();
                result << "<span class=\"code\">" << HTML::encode( std::string( match->first - CUT_OFF / 2, match->first ) ) << "</span>";
            } else {
                result << "<span class=\"code\">" << HTML::encode( std::string( from, match->first ) ) << "</span>";
            }

        }
        // else print code in neutral until end
        else {
            // elide right if line is too long
            if( line.cend() - from > CUT_OFF ) {
                result << "<span class=\"code\">" << HTML::encode( std::string( from, from + CUT_OFF ) ) << "</span>";
                this->ellipsis();
            } else {
                result << "<span class=\"code\">" << HTML::encode( std::string( from, line.cend() ) ) << "</span>\n";
            }
        }
    }

end:
    // close result
    result << "</div>\n\n";
}

void HtmlPrinter::printPrints() {
    fs::ofstream of( html, std::ios::out | std::ios::binary | std::ios::app );

    if( of ) {
        of << result.str();
    }
}
