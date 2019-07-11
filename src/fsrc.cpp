#include "searcher.hpp"
#include "prettyprinter.hpp"
#include "stopwatch.hpp"

int main( int argc, char* argv[] ) {

    StopWatch total;
    total.start();

    SearchOptions opts = SearchOptions::parseArgs( argc, argv );

    if( !opts ) { return EXIT_FAILURE; }

    if( !fs::exists( opts.path ) ) {
        printf( "Dir \"%s\" does not exist.\n", opts.path.string().c_str() );
        exit( -1 );
    }

    bool colorized = opts.colorized;
    Color gray = colorized ? Color::Gray : Color::Neutral;

    auto makePrinter = [colorized] {
        PrettyPrinter* printer = new PrettyPrinter();
        printer->opts.colorized = colorized;
        return printer;
    };

    Searcher searcher( opts, makePrinter );

    if( !opts.noGit && fs::exists( opts.path / ".git" ) ) {
        searcher.printGitHeader();
        searcher.onGitFiles();
    } else {
        searcher.printHeader();
        searcher.onAllFiles();
    }

    auto ms = total.stop() / 1000000;

#if DETAILED_STATS
    searcher.printStats();
#endif
    searcher.printFooter( ms );

    return 0;
}
