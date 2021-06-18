#include "searchcontroller.hpp"
#include "printer/printerfactory.hpp"
#include "searcher/searcherfactory.hpp"
#include "stopwatch.hpp"
#include "exitqueue.hpp"

int main( int argc, char* argv[] ) {

#if DETAILED_STATS
    StopWatch total;
    total.start();
#endif

    SearchOptions opts = SearchOptions::parseArgs( argc, argv );

    if( !opts ) { return EXIT_FAILURE; }

    // checks
    if( !fs::is_directory( opts.path ) ) {
        printf( "\"%s\" is not a directory.\n", opts.path.string().c_str() );
        exit( -1 );
    }

    std::function<Printer*()> makePrinter = printerfactory::printerFunc( opts );
    std::function<Searcher*()> makeSearcher = searcherfactory::searcherFunc( opts );
    SearchController searcher( opts, makeSearcher, makePrinter );

    if( !opts.noGit && fs::exists( opts.path / ".git" ) ) {
        // set prefix for clickable paths
        opts.pathPrefix = utils::absolutePath( opts.path.native() );

        searcher.onGitFiles();
    } else {
        searcher.onAllFiles();
    }

#if DETAILED_STATS
    auto ms = total.stop() / 1000000;
    searcher.printStats();
    searcher.printFooter( ms );
#endif

    ExitQueue::call();
    return 0;
}
