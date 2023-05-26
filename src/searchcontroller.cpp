
#include "threadpool.hpp"
#include "searchcontroller.hpp"
#include "printer/printer.hpp"
#include "searcher/searcher.hpp"

void SearchController::onAllFiles() {
    this->printHeader();

    POOL;
    STOPWATCH
    START

    utils::recurseDir( opts.path.native(), [&pool, this]( const sys_string & filename ) {
        pool.add( [filename{std::move( filename )}, this] {
            if( glob && !glob.matches( filename ) ) { return; }

#if DETAILED_STATS
            stats.filesSearched++;
#endif
            search( filename );
        } );
    } );

    STOP( stats.t_recurse )
}

void SearchController::onGitFiles() {
    this->printGitHeader();

    POOL;
    STOPWATCH
    START

    utils::gitLsFiles( opts.path, [&pool, this]( const sys_string & filename ) {
        pool.add( [filename{std::move( filename )}, this] {
            if( glob && !glob.matches( filename ) ) { return; }

#if DETAILED_STATS
            stats.filesSearched++;
#endif
            search( filename );
        } );
    } );

    STOP( stats.t_recurse );
}

void SearchController::printHeader() {
    if( !opts.piped ) {
        utils::printColor( gray, utils::format( "Searching for \"%s\" in folder:\n\n", opts.term.c_str() ) );
    }
}

void SearchController::printGitHeader() {
    if( !opts.piped ) {
        utils::printColor( gray, utils::format( "Searching for \"%s\" in git repo:\n\n", opts.term.c_str() ) );
    }
}

#if DETAILED_STATS
void SearchController::printStats() {
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

void SearchController::printFooter( const StopWatch::ns_type ms ) {
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
#endif

void SearchController::search( const sys_string& path ) {

    STOPWATCH
    START

    // read file
#ifndef _WIN32
    utils::FileView view = utils::fromFileP( path );
#else
    utils::FileView view = utils::fromWinAPI( path );
#endif

#if DETAILED_STATS
    stats.bytesRead += view.size;
#endif
    STOP( stats.t_read )

    if( !view.size ) { return; }

    // collect matches
    START

    const std::string_view& content = view.content;
    static thread_local std::unique_ptr<Searcher> searcher( makeSearcher() );
    std::vector<search::Match> matches = searcher->search( content );

    STOP( stats.t_search );

    // handle matches
    if( !matches.empty() ) {
#if DETAILED_STATS
        stats.filesMatched++;
        stats.matches += matches.size();
#endif

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
