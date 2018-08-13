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

            auto begin = rx::cregex_iterator( &line.front(), 1 + &line.back(), regex );
            auto end   = rx::cregex_iterator();

            if( std::distance( begin, end ) > 0 ) {
                prints.emplace_back( utils::printFunc( Color::Blue, "\nL%4i : ", i ) );
            }

            for( rx::cregex_iterator match = begin; match != end; ++match ) {
                hits++;
                prints.emplace_back( utils::printFunc( Color::Neutral, "%s", match->prefix().str().c_str() ) );
                prints.emplace_back( utils::printFunc( Color::Red, "%s", match->str().c_str() ) );

                if( std::distance( match, end ) == 1 ) {
                    prints.emplace_back( utils::printFunc( Color::Neutral, "%s", match->suffix().str().c_str() ) );
                }
            }
        }

        if( !prints.empty() ) {
            filesMatched++;
            prints.emplace_front( utils::printFunc( Color::Green, "%s", path.string().c_str() ) );
            prints.emplace_back( utils::printFunc( Color::Neutral, "%s", "\n\n" ) );
            m.lock();

            for( std::function<void()> func : prints ) { func(); }

            m.unlock();
        }
    }
};

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

    utils::recurseDirWin( directory, [&pool, &searcher]( const std::wstring & filename ) {
#if WITH_BOOST
        boost::asio::post( pool, [filename, &searcher] {
#else
        pool.add( [filename, &searcher] {
#endif
            searcher.files++;
            searcher.search( filename );
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
