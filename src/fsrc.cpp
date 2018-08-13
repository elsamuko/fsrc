#include <iostream>
#include <sstream>
#include <regex>

#include "threadpool.hpp"
#include "utils.hpp"

#if WITH_BOOST
#include "boost/regex.hpp"
#include "boost/asio.hpp"
namespace rx = boost;
#else
namespace rx = std;
#endif

#define LOG( A ) std::cout << A << std::endl;

struct Searcher {
    std::mutex m;
    rx::regex regex;
    std::atomic_int hits = 0;
    std::atomic_int files = 0;
    std::atomic_int filesMatched = 0;

    void search( const fs::path& path ) {

        // search only in text files
        if( !utils::isTextFile( path ) ) { return; }

        const std::pair<std::string, std::list<std::string_view>> lines = utils::fromFile( path );

        size_t i = 0;
        std::list<std::function<void()>> prints;

        for( const std::string_view& line : lines.second ) {
            i++;

            if( line.empty() ) { continue; }

            auto begin = rx::cregex_iterator( &line.front(), &line.back(), regex );
            auto end   = rx::cregex_iterator();

            if( std::distance( begin, end ) > 0 ) {
                prints.push_back( utils::printFunc( Color::Blue, "\nL%4i : ", i ) );
            }

            for( rx::cregex_iterator match = begin; match != end; ++match ) {
                hits++;
                prints.push_back( utils::printFunc( Color::Neutral, "%s", match->prefix().str().c_str() ) );
                prints.push_back( utils::printFunc( Color::Red, "%s", match->str().c_str() ) );

                if( std::distance( match, end ) == 1 ) {
                    prints.push_back( utils::printFunc( Color::Neutral, "%s", match->suffix().str().c_str() ) );
                }
            }
        }

        if( !prints.empty() ) {
            filesMatched++;
            prints.push_front( utils::printFunc( Color::Green, "%s", path.string().c_str() ) );
            prints.push_back( utils::printFunc( Color::Neutral, "%s", "\n\n" ) );
            m.lock();

            for( std::function<void()> func : prints ) { func(); }

            m.unlock();
        }
    }
};

void onAllFiles( const std::string directory, Searcher& searcher ) {

#if !WIN32
#if WITH_BOOST
    boost::asio::thread_pool pool( std::thread::hardware_concurrency() );
#else
    ThreadPool pool;
#endif

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

#if WITH_BOOST
    pool.join();
#endif

#else
    os::error_code ec;
    auto start = fs::recursive_directory_iterator( directory, ec );
    auto end   = fs::recursive_directory_iterator();

    if( ec ) {
        LOG( "Cannot recurse " << directory << " : " << ec.message() );
        return;
    }

    ThreadPool pool;

    while( start != end ) {

        fs::path path = start->path();
#if WITH_BOOST
        fs::file_status status = start.status();
#else
        fs::file_status status = fs::status( path );
#endif

        if( fs::is_directory( status ) && ( path.string().find( "/.git" ) != std::string::npos ) ) {
            start.disable_recursion_pending();
            start++;
#if WITH_BOOST
            status = start.status();
#else
            path = start->path();
            status = fs::status( path );
#endif
        }

        if( fs::is_regular_file( status ) ) {
            path = start->path();
            pool.add( [path, &searcher] {
                searcher.files++;
                searcher.search( path );
            } );
        }

        start++;
    }

#endif
}

void onGitFiles( const std::list<std::string>& filenames, Searcher& searcher ) {
    ThreadPool pool;

    for( const std::string& filename : filenames ) {
        fs::path path( filename );
        pool.add( [path, &searcher] {
            searcher.search( path );
        } );
    }
}

int main( int argc, char* argv[] ) {

    if( argc != 2 ) {
        LOG( "Usage: fscr <term>" );
        return 0;
    }

    std::string directory = ".";
    const std::string term = argv[1];
    auto tp = std::chrono::system_clock::now();

    Searcher searcher;

    try {
        searcher.regex = argv[1];
    } catch( const rx::regex_error& e ) {
        LOG( "Invalid regex: " << e.what() );
        return -1;
    }

#if WIN32
    std::string nullDevice = "NUL";
#else
    std::string nullDevice = "/dev/null";
#endif
    std::list<std::string> gitFiles = utils::run( "git ls-files 2> " + nullDevice );

    if( !gitFiles.empty() ) {
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
