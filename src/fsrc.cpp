#include "searcher.hpp"
#include "utils.hpp"

#define BOOST_THREADPOOL 1

#if !BOOST_THREADPOOL
#include "threadpool.hpp"
#else

#include <thread>
#include "boost/asio.hpp"

#define POOL struct ThreadPool { \
    boost::asio::thread_pool mPool{ std::min<size_t>( std::thread::hardware_concurrency(), 8u ) }; \
    void add( const std::function<void()>& f ) { \
        boost::asio::post( mPool, f ); \
    } \
    ThreadPool() {} \
    ~ThreadPool() { mPool.join(); } \
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
    printf( "Found %i hits in %i/%i files in %lld ms\n",
            searcher.hits.load(), searcher.filesMatched.load(),
            searcher.files.load(), ms.count() );

    return 0;
}
