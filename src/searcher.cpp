#include <iterator>

#include "searcher.hpp"

std::vector<Searcher::Hit> Searcher::caseInsensitiveSearch( const std::string_view& content ) {
    std::vector<Hit> hits;

    Iter pos = content.cbegin();
    Iter end = content.cend();
    const char* start = content.data();
    const char* ptr = start;

    while( pos != end ) {
        ptr = strcasestr( ptr, term.data() );

        if( ptr ) {
            Iter from = pos + ( ptr - start );
            Iter to = from + term.size();
            hits.emplace_back( from, to );
            ptr += term.size();
        } else {
            pos = end;
        }

    }

    return hits;
}

#if BOOST_OS_WINDOWS
std::vector<Searcher::Hit> Searcher::caseSensitiveSearch( const std::string_view& content ) {
    std::vector<Hit> hits;

    Iter pos = content.cbegin();
    Iter end = content.cend();
    const char* start = content.data();
    const char* ptr = start;

    while( pos != end ) {
        ptr = strstr( ptr, term.data() );

        if( ptr ) {
            Iter from = pos + ( ptr - start );
            Iter to = from + term.size();
            hits.emplace_back( from, to );
            ptr += term.size();
        } else {
            pos = end;
        }

    }

    return hits;
}
#else
std::vector<Searcher::Hit> Searcher::caseSensitiveSearch( const std::string_view& content ) {
    std::vector<Hit> hits;

    Iter pos = content.cbegin();
    Iter end = content.cend();
    size_t ptr = 0;

    while( pos != end ) {
        ptr = content.find( term, ptr );

        if( ptr != std::string_view::npos ) {
            Iter from = pos + ptr;
            Iter to = from + term.size();
            hits.emplace_back( from, to );
            ptr += term.size();
        } else {
            pos = end;
        }

    }

    return hits;
}
#endif

std::vector<Searcher::Hit> Searcher::regexSearch( const std::string_view& content ) {
    std::vector<Hit> hits;

    auto begin = rx::cregex_iterator( &content.front(), 1 + &content.back(), regex );
    auto end   = rx::cregex_iterator();

    for( rx::cregex_iterator match = begin; match != end; ++match ) {
        Iter from = content.cbegin() + match->position();
        Iter to = from + match->length();
        hits.emplace_back( from, to );
    }

    return hits;
}

std::vector<Searcher::Print> Searcher::collectPrints( const sys_string& path, const std::vector<Searcher::Hit>& hits, const std::string_view& content ) {
    std::vector<std::function<void()>> prints;
    prints.reserve( 3 * hits.size() );

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

    // parse file for newlines until last hit
    long long stop = hits.back().second - content.cbegin();
    utils::Lines lines = utils::parseContent( content.data(), content.size(), stop );

    std::vector<Hit>::const_iterator hit = hits.cbegin();
    size_t size = lines.size();

    for( size_t i = 0; i < size; ++i ) {
        Iter from = lines[i].cbegin();
        Iter to   = lines[i].cend();
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
    // print newlines
    prints.emplace_back( utils::printFunc( Color::Neutral, "\n\n" ) );

    return prints;
}

void Searcher::printPrints( const std::vector<Searcher::Print>& prints ) {
    std::unique_lock<std::mutex> lock( m );

    for( const std::function<void()>& func : prints ) { func(); }
}

void Searcher::search( const sys_string& path ) {

#ifndef _WIN32
    utils::FileView view = utils::fromFileC( path );
#else
    utils::FileView view = utils::fromWinAPI( path );
#endif
    const std::string_view& content = view.content;

    if( content.empty() ) { return; }

    std::vector<Hit> hits;
    std::vector<std::function<void()>> prints;

    // collect hits
    if( !opts.isRegex ) {
        if( opts.ignoreCase ) {
            hits = caseInsensitiveSearch( content );
        } else {
            hits = caseSensitiveSearch( content );
        }
    } else {
        hits = regexSearch( content );
    }

    // handle hits
    if( !hits.empty() ) {
        stats.filesMatched++;
        stats.count += hits.size();
        prints = collectPrints( path, hits, content );

        if( !opts.quiet ) { printPrints( prints );}
    }
}
