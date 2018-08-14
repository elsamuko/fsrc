#include "searcher.hpp"

#include "boost/program_options.hpp"
namespace po = boost::program_options;

SearchOptions SearchOptions::parseArgs( int argc, char* argv[] ) {
    SearchOptions opts;
    po::variables_map args;

    po::options_description desc( "Options" );
    desc.add_options()
    ( "help,h", "Help" )
    ( "dir,d", po::value<std::string>(), "Search folder" );

    po::options_description hidden( "Hidden options" );
    hidden.add_options()
    ( "term,t", po::value<std::string>()->required(), "Search term" );

    po::positional_options_description last;
    last.add( "term", -1 );

    po::options_description all;
    all.add( desc ).add( hidden );

    try {
        po::store( po::command_line_parser( argc, argv ).
                   options( all ).
                   positional( last ).
                   run(), args );
    } catch( const po::error& ex ) {
        LOG( "Error: " << ex.what() );
        opts.success = false;
    }

    // switch to dir
    if( args.count( "dir" ) ) {
        fs::path dir = args["dir"].as<std::string>();

        if( fs::exists( dir ) ) {
            opts.path = dir;
        }
    }

    // term
    if( args.count( "term" ) ) {
        opts.term = args["term"].as<std::string>();
        opts.success = true;
    } else {
        LOG( "Error: Missing term" );
        opts.success = false;
    }

    // help
    if( args.count( "help" ) ) {
        LOG( "fsrc [option] term" )
        desc.print( std::cout );
        opts.success = false;
    }

    return opts;
}

void Searcher::search( const fs::path& path, const size_t filesize ) {

    // search only in text files
    if( !utils::isTextFile( path ) ) { return; }

    const std::pair<std::string, std::list<std::string_view>> lines = utils::fromFile( path, filesize );

    size_t i = 0;
    std::list<std::function<void()>> prints;

    // don't pipe colors
    Color cred   = opts.colored ? Color::Red   : Color::Neutral;
    Color cblue  = opts.colored ? Color::Blue  : Color::Neutral;
    Color cgreen = opts.colored ? Color::Green : Color::Neutral;

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
