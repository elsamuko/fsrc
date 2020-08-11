#include "searchoptions.hpp"

#include "boost/program_options.hpp"
#include "boost/algorithm/string/replace.hpp"
namespace po = boost::program_options;

void usage( const std::string& description ) {
    LOG( "Usage  : fsrc [options] term" );
    LOG( description );
    LOG( "Build : " << GIT_TAG << " from " << __DATE__ );
    LOG( "Web   : https://github.com/elsamuko/fsrc" );
}

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
    ( "no-piped", "Disable piped output" )
    ( "html", "open web page with results" )
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
        LOG( "Error  : " << ex.what() );
        opts.success = false;

        std::stringstream help;
        desc.print( help );
        usage( help.str() );

        return opts;
    }

    // search in dir
    if( args.count( "dir" ) ) {
        std::string dir = args["dir"].as<std::string>();
#if BOOST_OS_WINDOWS

        boost::algorithm::replace_all( dir, "/", "\\" );

        if( dir.back() != '\\' ) {
            dir.push_back( '\\' );
        }

#else

        if( dir.back() != '/' ) {
            dir.push_back( '/' );
        }

#endif


        opts.path = utils::absolutePath( sys_string( dir.cbegin(), dir.cend() ) );
    } else {
        opts.path = utils::absolutePath();
    }

    // disable ls-files
    if( args.count( "no-git" ) ) {
        opts.noGit = true;
    }

    // disable piped output
    if( args.count( "no-piped" ) ) {
        opts.piped = false;
    }

    // disable colors
    if( args.count( "no-colors" ) ) {
        opts.colorized = false;
    }

    // don't print findings
    if( args.count( "quiet" ) ) {
        opts.quiet = true;
    }

    // print results to html
    if( args.count( "html" ) ) {
        opts.html = true;
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
        opts.success = !opts.term.empty();
    } else {
        opts.success = false;
    }

    // help
    if( args.count( "help" ) ) {
        opts.success = false;
    }

    if( !opts.success ) {
        std::stringstream help;
        desc.print( help );
        usage( help.str() );
    }

    return opts;
}
