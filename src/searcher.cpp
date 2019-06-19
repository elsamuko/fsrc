#include "searcher.hpp"


void Searcher::search( const sys_string& path ) {

    const utils::FileView view = utils::fromFileC( path );

    if( view.lines.empty() ) { return; }

    size_t size = view.lines.size();
    std::vector<std::function<void()>> prints;
    prints.reserve( 10 );

    // don't pipe colors
    Color cred   = opts.colorized ? Color::Red   : Color::Neutral;
    Color cblue  = opts.colorized ? Color::Blue  : Color::Neutral;
    Color cgreen = opts.colorized ? Color::Green : Color::Neutral;

    for( size_t i = 0; i < size; ++i ) {
        const std::string_view& line = view.lines[i];

        if( line.empty() ) { continue; }

        if( !opts.isRegex ) {

            size_t pos = std::string::npos;

            if( opts.ignoreCase ) {
                // strcasestr needs \0 to stop, string_view does not have that
                std::string copy( line.data(), line.size() );
                const char* ptr = strcasestr( copy.data(), term.data() );

                if( ptr ) { pos = ptr - copy.data(); }
                else { pos = std::string::npos; }
            } else {
                const auto res = ( *bmh )( line.begin(), line.end() );

                if( res.first != line.end() ) {
                    pos = res.first - line.begin();
                }
            }

            const char* data = line.data();

            // highlight only first hit
            if( pos != std::string::npos ) {
                hits++;
                std::string number = utils::format( "\nL%4i : ", i + 1 );
                prints.emplace_back( utils::printFunc( cblue, number ) );
                prints.emplace_back( utils::printFunc( Color::Neutral, std::string( data, pos ) ) );
                prints.emplace_back( utils::printFunc( cred, std::string( data + pos, term.size() ) ) );
                std::string rest( data + pos + term.size(), line.size() - pos - term.size() );
                prints.emplace_back( utils::printFunc( Color::Neutral, rest ) );
            }

        } else {
            auto begin = rx::cregex_iterator( &line.front(), 1 + &line.back(), regex );
            auto end   = rx::cregex_iterator();

            if( std::distance( begin, end ) > 0 ) {
                std::string number = utils::format( "\nL%4i : ", i + 1 );
                prints.emplace_back( utils::printFunc( cblue, number ) );
            } else {
                continue;
            }

            for( rx::cregex_iterator match = begin; match != end; ++match ) {
                hits++;
                prints.emplace_back( utils::printFunc( Color::Neutral, match->prefix().str() ) );
                prints.emplace_back( utils::printFunc( cred, match->str() ) );

                if( std::distance( match, end ) == 1 ) {
                    prints.emplace_back( utils::printFunc( Color::Neutral, match->suffix().str() ) );
                }
            }
        }
    }


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
