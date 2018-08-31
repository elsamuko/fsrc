#include "searcher.hpp"
#include "utils.hpp"

#include "threadpool.hpp"

#define THREADED 1

#if THREADED

// max 8 threads, else start/stop needs longer than the actual work
#define POOL ThreadPool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

#else

#define POOL struct { \
    void add( const std::function<void()>& f ) { \
        f(); \
    } \
    } pool;

#endif

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

    SearchOptions opts = SearchOptions::parseArgs( argc, argv );

    if( !opts ) { return -1; }

    auto tp = std::chrono::system_clock::now();

    Searcher searcher( opts );

    if( !opts.noGit && fs::exists( opts.path / ".git" ) ) {
#if WIN32
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
    printf( "Found %i hits in %i/%i files in %lld ms\n",
            searcher.hits.load(), searcher.filesMatched.load(),
            searcher.files.load(), ms.count() );

    return 0;
}
