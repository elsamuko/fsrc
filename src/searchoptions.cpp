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
    ( "dir,d", po::value<std::string>(), "Search folder" )
    ( "ext,e", po::value<std::string>(), "Search only in files with extension <arg>, equiv. to --glob '*.ext'" )
    ( "files,f", "Only print filenames" )
    ( "glob,g", po::value<std::string>(), "Search only in files filtered by <arg> glob, e.g. '*.txt'; overrides --ext" )
    ( "help,h", "Help" )
    ( "html", "open web page with results" )
    ( "ignore-case,i", "Case insensitive search" )
    ( "no-git", "Disable search with 'git ls-files'" )
    ( "no-colors", "Disable colorized output" )
    ( "no-piped", "Disable piped output" )
    ( "no-uri", "Print w/out file:// prefix" )
    ( "piped", "Enable piped output" )
    ( "quiet,q", "only print status" )
    ( "regex,r", "Regex search (slower)" )
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

    // enable piped output
    if( args.count( "piped" ) ) {
        opts.piped = true;
    }

    // disable piped output
    if( args.count( "no-piped" ) ) {
        opts.piped = false;
    }

    // disable colors
    if( args.count( "no-colors" ) ) {
        opts.colorized = false;
    }

    // disable file:// prefix
    if( args.count( "no-uri" ) ) {
        opts.noURI = true;
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

    // filter by extension
    if( args.count( "ext" ) ) {
        opts.glob = "*." + args["ext"].as<std::string>();
    }

    // filter by glob
    if( args.count( "glob" ) ) {
        opts.glob = args["glob"].as<std::string>();
    }

    // only print filenames
    if( args.count( "files" ) ) {
        opts.onlyFiles = true;
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
