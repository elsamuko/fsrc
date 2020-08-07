#include "searcher.hpp"
#include "printer/printerfactory.hpp"
#include "stopwatch.hpp"
#include "exitqueue.hpp"

int main( int argc, char* argv[] ) {

    StopWatch total;
    total.start();

    SearchOptions opts = SearchOptions::parseArgs( argc, argv );

    if( !opts ) { return EXIT_FAILURE; }

    // checks
    if( !fs::is_directory( opts.path ) ) {
        printf( "\"%s\" is not a directory.\n", opts.path.string().c_str() );
        exit( -1 );
    }

    std::function<Printer*()> makePrinter = printerfactory::printerFunc( opts );
    Searcher searcher( opts, makePrinter );

    if( !opts.noGit && fs::exists( opts.path / ".git" ) ) {
        // set prefix for clickable paths
        opts.prefix = utils::absolutePath( opts.path.native() );

        searcher.onGitFiles();
    } else {
        // set prefix for clickable paths
        opts.prefix = utils::absolutePath();

        searcher.onAllFiles();
    }

    auto ms = total.stop() / 1000000;

#if DETAILED_STATS
    searcher.printStats();
#endif
    searcher.printFooter( ms );

    ExitQueue::call();
    return 0;
}
