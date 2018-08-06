#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <fstream>
#include <algorithm>
#include <experimental/filesystem>

#include "threadpool.hpp"

#define LOG( A ) std::cout << A << std::endl;

std::list<std::string> fromFile( const std::experimental::filesystem::path& filename ) {
    std::list<std::string> lines;
    std::ifstream file( filename.c_str(), std::ios::binary );

    if( !file ) { return lines;}

    file.seekg( 0, std::ios::end );
    size_t length = ( size_t ) file.tellg();
    file.seekg( 0, std::ios::beg );

    const std::string content( length, '\0' );
    file.read( ( char* ) content.data(), length );

    int pos = 0;

    for( size_t i = 0; i < length; ++i ) {
        // just skip windows line endings
        if( content[i] == '\r' ) {
            ++i;
        }

        // cut at unix line endings
        if( content[i] == '\n' ) {
            lines.push_back( content.substr( pos, i - pos ) );
            pos = i + 1;
        }
    }

    return lines;
}

void onAllFiles( const std::experimental::filesystem::path searchpath, const std::function<void( const std::experimental::filesystem::path& )>& func ) {

    if( !std::experimental::filesystem::exists( searchpath ) ) {
        LOG( searchpath << " does not exist" );
        return;
    }

    if( !std::experimental::filesystem::is_directory( searchpath ) ) {
        LOG( searchpath << " is not a dir" );
        return;
    }

    std::for_each( std::experimental::filesystem::recursive_directory_iterator( searchpath ), std::experimental::filesystem::recursive_directory_iterator(), func );
}

template<class C, class T>
bool contains( const C& v, const T& x ) {
    return std::end( v ) != std::find( begin( v ), end( v ), x );
}

int main( int argc, char* argv[] ) {

    if( argc != 2 ) {
        LOG( "Usage: fscr <term>" );
        return 0;
    }

    ThreadPool pool;
    std::mutex m;
    std::experimental::filesystem::path searchpath = ".";
    const std::string term = argv[1];
    auto tp = std::chrono::system_clock::now();
    LOG( "Searching for \"" << term << "\":\n" );

    onAllFiles( searchpath, [&m, &pool, &term]( const std::experimental::filesystem::path & path ) {
        pool.add( [&m, path, &term] {

            std::experimental::filesystem::path ext = path.extension();
            const std::vector<std::experimental::filesystem::path> exts = { ".cpp", ".cc", ".h", ".hpp", ".pro", ".pri", ".py", ".prf", ".conf" };

            if( contains( exts, ext ) ) {
                const std::list<std::string> lines = fromFile( path );

                std::stringstream ss;
                size_t i = 0;

                for( const std::string& line : lines ) {
                    i++;

                    size_t pos = line.find( term );

                    if( pos != std::string::npos ) {
                        ss << path << " L" << i << " " << line.substr( 0, pos );
#if WIN32
                        ss << line.substr( pos, term.size() );
#else
                        // highlight first hit on linux
                        ss << "\033[1;31m" << line.substr( pos, term.size() ) << "\033[0m";
#endif
                        ss << line.substr( pos  + term.size() );
                        ss << std::endl;
                    }
                }

                std::string res = ss.str();

                if( !res.empty() ) {
                    m.lock();
                    LOG( res );
                    m.unlock();
                }
            }

        } );
    } );

    pool.waitForAllJobs();

    auto duration = std::chrono::system_clock::now() - tp;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>( duration );
    LOG( "Searched in " << ms.count() << " ms" );

    return 0;
}
