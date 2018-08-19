#include "searcher.hpp"
#include "utils.hpp"

#define THREADED 1

#if THREADED

#include "boost/asio.hpp"
using boost::asio::post;
// max 8 threads, else start/stop needs longer than the actual work
#define POOL boost::asio::thread_pool pool( std::min( std::thread::hardware_concurrency(), 8u ) );\
    utils::ScopeGuard onExit( [&pool] { pool.join(); } )

#else

#define POOL int pool = 0
inline void post( const int, const std::function<void()>& f ) {
    f();
}

#endif

void onAllFiles( Searcher& searcher ) {
    POOL;

    utils::recurseDir( searcher.opts.path.native(), [&pool, &searcher]( const sys_string & filename ) {
        post( pool, [filename, &searcher] {
            searcher.files++;
            searcher.search( filename );
        } );
    } );
}

void onGitFiles( const std::vector<std::string>& filenames, Searcher& searcher ) {
    POOL;

    for( const std::string& filename : filenames ) {
        post( pool, [filename, &searcher] {
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
        std::vector<std::string> gitFiles = utils::run( "git ls-files 2> " + nullDevice );
        LOG( "Searching for \"" << searcher.opts.term << "\" in repo:\n" );
        onGitFiles( gitFiles, searcher );
    } else {
        LOG( "Searching for \"" << searcher.opts.term << "\" in folder:\n" );
        onAllFiles( searcher );
    }

    auto duration = std::chrono::system_clock::now() - tp;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>( duration );
    LOG( "Found " << searcher.hits << " hits in "
         << searcher.filesMatched << "/" << searcher.files
         << " files in " << ms.count() << " ms" );

    return 0;
}
