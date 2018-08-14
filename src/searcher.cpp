#include "searcher.hpp"

void Searcher::search( const fs::path& path, const size_t filesize ) {

    // search only in text files
    if( !utils::isTextFile( path ) ) { return; }

    const std::pair<std::string, std::list<std::string_view>> lines = utils::fromFile( path, filesize );

    size_t i = 0;
    std::list<std::function<void()>> prints;

    // don't pipe colors
    Color cred   = colored ? Color::Red   : Color::Neutral;
    Color cblue  = colored ? Color::Blue  : Color::Neutral;
    Color cgreen = colored ? Color::Green : Color::Neutral;

    for( const std::string_view& line : lines.second ) {
        i++;

        if( line.empty() ) { continue; }

        auto begin = rx::cregex_iterator( &line.front(), 1 + &line.back(), regex );
        auto end   = rx::cregex_iterator();

        if( std::distance( begin, end ) > 0 ) {
            prints.emplace_back( utils::printFunc( cblue, "\nL%4i : ", i ) );
        }

        for( rx::cregex_iterator match = begin; match != end; ++match ) {
            hits++;
            prints.emplace_back( utils::printFunc( Color::Neutral, "%s", match->prefix().str().c_str() ) );
            prints.emplace_back( utils::printFunc( cred, "%s", match->str().c_str() ) );

            if( std::distance( match, end ) == 1 ) {
                prints.emplace_back( utils::printFunc( Color::Neutral, "%s", match->suffix().str().c_str() ) );
            }
        }
    }

    if( !prints.empty() ) {
        filesMatched++;
        prints.emplace_front( utils::printFunc( cgreen, "%s", path.string().c_str() ) );
        prints.emplace_back( utils::printFunc( Color::Neutral, "%s", "\n\n" ) );
        m.lock();

        for( std::function<void()> func : prints ) { func(); }

        m.unlock();
    }
}
