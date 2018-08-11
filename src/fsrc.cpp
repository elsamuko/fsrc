#include <iostream>
#include <sstream>
#include <regex>

#include "threadpool.hpp"
#include "utils.hpp"

#if WITH_BOOST
#include <boost/regex.hpp>
namespace rx = boost;
#else
namespace rx = std;
#endif

#define LOG( A ) std::cout << A << std::endl;

struct Searcher {
    mutable std::mutex m;
    rx::regex regex;
    void search( const std::experimental::filesystem::path& path ) const {
        // search only in text files
        if( !utils::isTextFile( path ) ) { return; }

        const std::list<std::string> lines = utils::fromFile( path );

        size_t i = 0;
        std::list<std::function<void()>> prints;

        for( const std::string& line : lines ) {
            i++;

            if( line.empty() ) { continue; }

            auto begin = rx::cregex_iterator( &line.front(), &line.back(), regex );
            auto end   = rx::cregex_iterator();

            if( std::distance( begin, end ) > 0 ) {
                prints.push_back( utils::printFunc( Color::Blue, "\nL%4i : ", i ) );
            }

            for( rx::cregex_iterator match = begin; match != end; ++match ) {
                prints.push_back( utils::printFunc( Color::Neutral, "%s", match->prefix().str().c_str() ) );
                prints.push_back( utils::printFunc( Color::Red, "%s", match->str().c_str() ) );

                if( std::distance( match, end ) == 1 ) {
                    prints.push_back( utils::printFunc( Color::Neutral, "%s", match->suffix().str().c_str() ) );
                }
            }
        }

        if( !prints.empty() ) {
            prints.push_front( utils::printFunc( Color::Green, "%s", path.string().c_str() ) );
            prints.push_back( utils::printFunc( Color::Neutral, "%s", "\n\n" ) );
            m.lock();

            for( std::function<void()> func : prints ) { func(); }

            m.unlock();
        }
    }
};

void onAllFiles( const std::experimental::filesystem::path searchpath, const Searcher& searcher ) {

    if( !std::experimental::filesystem::exists( searchpath ) ) {
        LOG( searchpath << " does not exist" );
        return;
    }

    if( !std::experimental::filesystem::is_directory( searchpath ) ) {
        LOG( searchpath << " is not a dir" );
        return;
    }

    auto start = std::experimental::filesystem::recursive_directory_iterator( searchpath );
    auto end   = std::experimental::filesystem::recursive_directory_iterator();

    ThreadPool pool;

    while( start != end ) {

        std::experimental::filesystem::path path = start->path();

        if( std::experimental::filesystem::is_directory( path ) &&
                ( path.string().find( ".git" ) != std::string::npos ) ) {
            start.disable_recursion_pending();
            start++;
            path = start->path();
        }

        pool.add( [path, &searcher] {
            searcher.search( path );
        } );
        start++;
    }

    pool.waitForAllJobs();
}

void onGitFiles( const std::list<std::string>& filenames, const Searcher& searcher ) {
    ThreadPool pool;

    for( const std::string& filename : filenames ) {
        std::experimental::filesystem::path path( filename );
        pool.add( [path, &searcher] {
            searcher.search( path );
        } );
    }

    pool.waitForAllJobs();
}

int main( int argc, char* argv[] ) {

    if( argc != 2 ) {
        LOG( "Usage: fscr <term>" );
        return 0;
    }

    std::experimental::filesystem::path searchpath = ".";
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
        onAllFiles( searchpath, searcher );
    }

    auto duration = std::chrono::system_clock::now() - tp;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>( duration );
    LOG( "Searched in " << ms.count() << " ms" );

    return 0;
}
