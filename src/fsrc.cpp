#include "threadpool.hpp"
#include "searcher.hpp"
#include "utils.hpp"

#if WITH_BOOST
#include "boost/asio.hpp"
#endif

void onAllFiles( const fs::path::string_type directory, Searcher& searcher ) {

#if WITH_BOOST
    boost::asio::thread_pool pool( std::thread::hardware_concurrency() );
#else
    ThreadPool pool;
#endif

#if !WIN32
    utils::recurseDirUnix( directory, [&pool, &searcher]( const std::string & filename ) {
#if WITH_BOOST
        boost::asio::post( pool, [filename, &searcher] {
#else
        pool.add( [filename, &searcher] {
#endif
            searcher.files++;
            searcher.search( filename );
        } );
    } );

#else

    utils::recurseDirWin( directory, [&pool, &searcher]( const std::wstring & filename, const size_t filesize ) {
#if WITH_BOOST
        boost::asio::post( pool, [filename, filesize, &searcher] {
#else
        pool.add( [filename, filesize, &searcher] {
#endif
            searcher.files++;
            searcher.search( filename, filesize );
        } );
    } );
#endif

#if WITH_BOOST
    pool.join();
#endif
}

void onGitFiles( const std::list<std::string>& filenames, Searcher& searcher ) {
    ThreadPool pool;

    for( const std::string& filename : filenames ) {
        fs::path path( filename );
        pool.add( [path, &searcher] {
            searcher.files++;
            searcher.search( path );
        } );
    }
}

int main( int argc, char* argv[] ) {

    if( argc != 2 ) {
        LOG( "Usage: fscr <term>" );
        return 0;
    }

    fs::path::string_type directory = fs::current_path().c_str();
    const std::string term = argv[1];
    auto tp = std::chrono::system_clock::now();

    Searcher searcher;

    try {
        searcher.regex = argv[1];
    } catch( const rx::regex_error& e ) {
        LOG( "Invalid regex: " << e.what() );
        return -1;
    }

    if( fs::exists( ".git" ) ) {
#if WIN32
        std::string nullDevice = "NUL";
#else
        std::string nullDevice = "/dev/null";
#endif
        std::list<std::string> gitFiles = utils::run( "git ls-files 2> " + nullDevice );
        LOG( "Searching for \"" << term << "\" in repo:\n" );
        onGitFiles( gitFiles, searcher );
    } else {
        LOG( "Searching for \"" << term << "\" in folder:\n" );
        onAllFiles( directory, searcher );
    }

    auto duration = std::chrono::system_clock::now() - tp;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>( duration );
    LOG( "Found " << searcher.hits << " hits in "
         << searcher.filesMatched << "/" << searcher.files
         << " files in " << ms.count() << " ms" );

    return 0;
}
