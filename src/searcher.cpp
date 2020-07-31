#include <iterator>

#include "threadpool.hpp"
#include "searcher.hpp"
#include "mischasan.hpp"
#include "stdstr.hpp"
#include "printer/printer.hpp"

std::vector<search::Match> Searcher::caseInsensitiveSearch( const std::string_view& content ) {
    std::vector<search::Match> matches;

    search::Iter pos = content.cbegin();
    const char* start = content.data();
    const char* ptr = start;

    while( ( ptr = strcasestr( ptr, term.data() ) ) ) {
        search::Iter from = pos + ( ptr - start );
        search::Iter to = from + term.size();
        matches.emplace_back( from, to );
        ptr += term.size();
    }

    return matches;
}

#if BOOST_OS_WINDOWS
std::vector<search::Match> Searcher::caseSensitiveSearch( const std::string_view& content ) {
    std::vector<search::Match> matches;

    search::Iter pos = content.cbegin();
    const char* start = content.data();
    const char* ptr = start;

    while( ( ptr = strstr( ptr, term.data() ) ) ) {
        search::Iter from = pos + ( ptr - start );
        search::Iter to = from + term.size();
        matches.emplace_back( from, to );
        ptr += term.size();
    }

    return matches;
}
#else
std::vector<search::Match> Searcher::caseSensitiveSearch( const std::string_view& content ) {
    std::vector<search::Match> matches;

    search::Iter pos = content.cbegin();
    const char* start = content.data();
    const char* ptr = start;
    const char* end = start + content.size();

#if WITH_SSE

    while( ( ptr = mischasan::scanstrN( ptr, end - ptr, term.data(), term.size() ) ) )
#else

    while( ( ptr = fromStd::strstr( ptr, end - ptr, term.data(), term.size() ) ) )
#endif

    {
        search::Iter from = pos + ( ptr - start );
        search::Iter to = from + term.size();
        matches.emplace_back( from, to );
        ptr += term.size();
    }

    return matches;
}
#endif

std::vector<search::Match> Searcher::regexSearch( const std::string_view& content ) {
    std::vector<search::Match> matches;

    // https://www.boost.org/doc/libs/1_70_0/libs/regex/doc/html/boost_regex/ref/match_flag_type.html
    rx::regex_constants::match_flags flags = rx::regex_constants::match_not_dot_newline;

    auto begin = rx::cregex_iterator( &content.front(), 1 + &content.back(), regex, flags );
    auto end   = rx::cregex_iterator();

    for( rx::cregex_iterator match = begin; match != end; ++match ) {
        search::Iter from = content.cbegin() + match->position();
        search::Iter to = from + match->length();
        matches.emplace_back( from, to );
    }

    return matches;
}

void Searcher::onAllFiles() {
    this->printHeader();

    POOL;
    STOPWATCH
    START

    utils::recurseDir( opts.path.native(), [&pool, this]( const sys_string & filename ) {
        pool.add( [filename, this] {
            stats.filesSearched++;
            search( filename );
        } );
    } );

    STOP( stats.t_recurse )
}

void Searcher::onGitFiles() {
    this->printGitHeader();

    POOL;
    STOPWATCH
    START

    utils::gitLsFiles( opts.path, [&pool, this]( const sys_string & filename ) {
        pool.add( [filename, this] {
            stats.filesSearched++;
            search( filename );
        } );
    } );

    STOP( stats.t_recurse );
}

void Searcher::printHeader() {
    if( !opts.piped ) {
        utils::printColor( gray, utils::format( "Searching for \"%s\" in folder:\n\n", opts.term.c_str() ) );
    }
}

void Searcher::printGitHeader() {
    if( !opts.piped ) {
        utils::printColor( gray, utils::format( "Searching for \"%s\" in git repo:\n\n", opts.term.c_str() ) );
    }
}

void Searcher::printStats() {
    if( !opts.piped ) {
        utils::printColor( gray, utils::format(
                               "Times: Recurse %ld ms, Read %ld ms, Search %ld ms, Collect %ld ms, Print %ld ms\n",
                               stats.t_recurse / 1000000,
                               stats.t_read / 1000000,
                               stats.t_search / 1000000,
                               stats.t_collect / 1000000,
                               stats.t_print / 1000000 ) );

    }
}

void Searcher::printFooter( const StopWatch::ns_type& ms ) {
    if( !opts.piped ) {
        utils::printColor( gray, utils::format(
                               "Found %lu matches in %lu/%lu files (%lu kB) in %ld ms\n",
                               stats.matches.load(),
                               stats.filesMatched.load(),
                               stats.filesSearched.load(),
                               stats.bytesRead.load() / 1024,
                               ms ) );
    }
}

void Searcher::search( const sys_string& path ) {

    STOPWATCH
    START

#ifndef _WIN32
    utils::FileView view = utils::fromFileP( path );
#else
    utils::FileView view = utils::fromWinAPI( path );
#endif

    stats.bytesRead += view.size;
    STOP( stats.t_read )

    if( !view.size ) { return; }

    // collect matches
    START
    const std::string_view& content = view.content;
    std::vector<search::Match> matches;

    if( !opts.isRegex ) {
        if( opts.ignoreCase ) {
            matches = caseInsensitiveSearch( content );
        } else {
            matches = caseSensitiveSearch( content );
        }
    } else {
        matches = regexSearch( content );
    }

    STOP( stats.t_search );

    // handle matches
    if( !matches.empty() ) {
        stats.filesMatched++;
        stats.matches += matches.size();

        START
        static thread_local std::unique_ptr<Printer> printer( makePrinter() );
        printer->collectPrints( path, matches, content );
        STOP( stats.t_collect );

        if( !opts.quiet ) {
            START
            std::unique_lock<std::mutex> lock( m );
            printer->printPrints();
            STOP( stats.t_print );
        }
    }
}
