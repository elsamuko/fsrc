#include "searcher.hpp"

#include "boost/program_options.hpp"
namespace po = boost::program_options;

SearchOptions SearchOptions::parseArgs( int argc, char* argv[] ) {
    SearchOptions opts;
    po::variables_map args;

    po::options_description desc( "Options" );
    desc.add_options()
    ( "help,h", "Help" )
    ( "dir,d", po::value<std::string>(), "Search folder" )
    ( "ignore-case,i", "Case insensitive search" )
    ( "regex,r", "Regex search (slower)" )
    ( "no-git", "Disable search with 'git ls-files'" )
    ( "no-colors", "Disable colorized output" )
    ;

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

    // search in dir
    if( args.count( "dir" ) ) {
        fs::path dir = args["dir"].as<std::string>();

        if( fs::exists( dir ) ) {
            opts.path = dir;
        }
    } else {
        opts.path = fs::current_path();
    }

    // disable ls-files
    if( args.count( "no-git" ) ) {
        opts.noGit = true;
    }

    // disable colors
    if( args.count( "no-colors" ) ) {
        opts.colorized = false;
    }

    // ignore case
    if( args.count( "ignore-case" ) ) {
        opts.ignoreCase = true;
    }

    // enable regex search
    if( args.count( "regex" ) ) {
        opts.isRegex = true;
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
                pos = line.find( term );
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

#if WIN32
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
