#include <iterator>

#include "searcher.hpp"

void Searcher::search( const sys_string& path ) {

    utils::FileView view = utils::fromFileC( path );
    const std::string_view& content = view.content;

    if( content.empty() ) { return; }

    std::vector<Hit> hits;
    std::vector<std::function<void()>> prints;

    // don't pipe colors
    Color cred   = opts.colorized ? Color::Red   : Color::Neutral;
    Color cblue  = opts.colorized ? Color::Blue  : Color::Neutral;
    Color cgreen = opts.colorized ? Color::Green : Color::Neutral;

    // collect hits
    if( !opts.isRegex ) {

        // size_t pos = std::string::npos;
        Iter pos = content.cbegin();
        Iter end = content.cend();

        while( pos != end ) {
            if( opts.ignoreCase ) {
                // strcasestr needs \0 to stop, string_view does not have that
                std::string copy( pos, end );
                const char* ptr = strcasestr( copy.data(), term.data() );

                if( ptr ) {
                    Iter from = pos + ( ptr - copy.data() );
                    Iter to = from + term.size();
                    hits.emplace_back( from, to );
                    pos = to;
                } else {
                    pos = end;
                }

            } else {
                const Hit hit = ( *bmh )( pos, end );

                if( hit.first != end ) {
                    hits.emplace_back( hit );
                    pos = hit.second;
                } else {
                    pos = end;
                }
            }
        }

    } else {
        auto begin = rx::cregex_iterator( &content.front(), 1 + &content.back(), regex );
        auto end   = rx::cregex_iterator();

        for( rx::cregex_iterator match = begin; match != end; ++match ) {
            Iter from = content.cbegin() + match->position();
            Iter to = from + match->length();
            hits.emplace_back( from, to );
        }
    }

    // handle hits
    if( !hits.empty() ) {
        prints.reserve( 10 );
        count += hits.size();

        size_t stop = hits.back().second - content.cbegin();
        view.lines = utils::parseContent( content.data(), content.size(), stop );
        size_t size = view.lines.size();

        std::vector<Hit>::const_iterator hit = hits.cbegin();

        for( size_t i = 0; i < size; ++i ) {
            Iter from = view.lines[i].cbegin();
            Iter to   = view.lines[i].cend();
            bool printedLine = false;

            // find all hits in this line
            while( from <= hit->first && hit->first < to ) {

                // print line information in blue once per line
                if( !printedLine ) {
                    printedLine = true;
                    std::string number = utils::format( "\nL%4i : ", i + 1 );
                    prints.emplace_back( utils::printFunc( cblue, number ) );
                }

                // print code in neutral
                prints.emplace_back( utils::printFunc( Color::Neutral, std::string( from, hit->first ) ) );

                // print hit in red
                prints.emplace_back( utils::printFunc( cred, std::string( hit->first, hit->second ) ) );

                // set from to end of hit
                from = hit->second;

                // if there are no more hits in this file, print rest of line in neutral
                // and exit search for this file
                if( std::next( hit ) == hits.cend() ) {
                    prints.emplace_back( utils::printFunc( Color::Neutral, std::string( from, to ) ) );
                    goto end;
                }

                ++hit;

                // if next hit is within this line, print code in neutral until next hit
                if( from <= hit->first && hit->first < to ) {
                    prints.emplace_back( utils::printFunc( Color::Neutral, std::string( from, hit->first ) ) );
                    from = hit->first;
                }
                // else print code in neutral until end and break to next line
                else {
                    prints.emplace_back( utils::printFunc( Color::Neutral, std::string( from, to ) ) );
                    break;
                }
            }
        }

end:
        // do nothing
        void();
    }

    // print hits
    if( !prints.empty() ) {
        filesMatched++;

#ifdef _WIN32
        const auto printFile     = utils::printFunc( cgreen, std::string( path.cbegin(), path.cend() ) );
#else
        const auto printFile     = utils::printFunc( cgreen, path );
#endif
        const auto printNewlines = utils::printFunc( Color::Neutral, "\n\n" );

        m.lock();
        printFile();

        for( const std::function<void()>& func : prints ) { func(); }

        printNewlines();
        m.unlock();
    }
}
