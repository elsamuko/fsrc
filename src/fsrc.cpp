#include "searcher.hpp"
#include "utils.hpp"

#include "boost/asio.hpp"

void onAllFiles( Searcher& searcher ) {

    // max 8 threads, else start/stop needs longer than the actual work
    boost::asio::thread_pool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

    utils::recurseDir( searcher.opts.path.native(), [&pool, &searcher]( const sys_string & filename ) {
        boost::asio::post( pool, [filename, &searcher] {
            searcher.files++;
            searcher.search( filename );
        } );
    } );

    pool.join();
}

void onGitFiles( const std::list<std::string>& filenames, Searcher& searcher ) {
    // max 8 threads, else start/stop needs longer than the actual work
    boost::asio::thread_pool pool( std::min( std::thread::hardware_concurrency(), 8u ) );

    for( const std::string& filename : filenames ) {
        boost::asio::post( pool, [filename, &searcher] {
            searcher.files++;
            searcher.search( filename );
        } );
    }

    pool.join();
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
        std::list<std::string> gitFiles = utils::run( "git ls-files 2> " + nullDevice );
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
