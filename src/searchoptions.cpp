#include "searchoptions.hpp"

#include "boost/program_options.hpp"
namespace po = boost::program_options;

void usage() {
    LOG( "Build: " __DATE__ );
    LOG( "Usage: fsrc [options] term" );
}

SearchOptions SearchOptions::parseArgs( int argc, char* argv[] ) {
    SearchOptions opts;
    po::variables_map args;
    bool needsHelp = false;

    po::options_description desc( "Options" );
    desc.add_options()
    ( "help,h", "Help" )
    ( "dir,d", po::value<std::string>(), "Search folder" )
    ( "ignore-case,i", "Case insensitive search" )
    ( "regex,r", "Regex search (slower)" )
    ( "no-git", "Disable search with 'git ls-files'" )
    ( "no-colors", "Disable colorized output" )
    ( "quiet,q", "only print status" )
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

    // don't print findings
    if( args.count( "quiet" ) ) {
        opts.quiet = true;
    }

    // ignore case
    if( args.count( "ignore-case" ) ) {
        opts.ignoreCase = true;
    }

    // enable regex search
    if( args.count( "regex" ) ) {
        opts.isRegex = true;
    }

    // help
    if( args.count( "help" ) ) {
        usage();
        desc.print( std::cout );
        needsHelp = true;
        opts.success = false;
    }

    // term
    if( args.count( "term" ) ) {
        opts.term = args["term"].as<std::string>();
        opts.success = true;
    } else {
        if( !needsHelp ) {
            usage();
        }

        opts.success = false;
    }

    return opts;
}
