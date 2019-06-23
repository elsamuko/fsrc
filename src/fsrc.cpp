#include "threadpool.hpp"
#include "searcher.hpp"
#include "utils.hpp"

void onAllFiles( Searcher& searcher ) {
    POOL;

    utils::recurseDir( searcher.opts.path.native(), [&pool, &searcher]( const sys_string & filename ) {
        pool.add( [filename, &searcher] {
            searcher.files++;
            searcher.search( filename );
        } );
    } );
}

void onGitFiles( const std::vector<sys_string>& filenames, Searcher& searcher ) {
    POOL;

    for( const sys_string& filename : filenames ) {
        pool.add( [filename, &searcher] {
            searcher.files++;
            searcher.search( filename );
        } );
    }
}

int main( int argc, char* argv[] ) {

    auto tp = std::chrono::system_clock::now();

    SearchOptions opts = SearchOptions::parseArgs( argc, argv );

    if( !opts ) { return EXIT_FAILURE; }

    Searcher searcher( opts );

    if( !opts.noGit && fs::exists( opts.path / ".git" ) ) {
#ifdef _WIN32
        std::string nullDevice = "NUL";
#else
        std::string nullDevice = "/dev/null";
#endif
        fs::current_path( opts.path );
        std::vector<sys_string> gitFiles = utils::run( "git ls-files 2> " + nullDevice );
        printf( "Searching for \"%s\" in repo:\n\n", searcher.opts.term.c_str() );
        onGitFiles( gitFiles, searcher );
    } else {
        printf( "Searching for \"%s\" in folder:\n\n", searcher.opts.term.c_str() );
        onAllFiles( searcher );
    }

    auto duration = std::chrono::system_clock::now() - tp;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>( duration );
    printf( "Found %lu hits in %lu/%lu files in %ld ms\n",
            searcher.count.load(), searcher.filesMatched.load(),
            searcher.files.load(), ms.count() );

    return 0;
}
