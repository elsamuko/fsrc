#include "searcher.hpp"
#include "printerfactory.hpp"
#include "stopwatch.hpp"
#include "exitqueue.hpp"

int main( int argc, char* argv[] ) {

    StopWatch total;
    total.start();

    SearchOptions opts = SearchOptions::parseArgs( argc, argv );

    if( !opts ) { return EXIT_FAILURE; }

    if( !fs::exists( opts.path ) ) {
        printf( "Dir \"%s\" does not exist.\n", opts.path.string().c_str() );
        exit( -1 );
    }

    std::function<Printer*()> makePrinter = printerfactory::printerFunc( opts );
    Searcher searcher( opts, makePrinter );

    if( !opts.noGit && fs::exists( opts.path / ".git" ) ) {
        searcher.onGitFiles();
    } else {
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
