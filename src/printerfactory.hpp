#pragma once

#include "prettyprinter.hpp"
#include "pipedprinter.hpp"
#include "searchoptions.hpp"

namespace printerfactory {

std::function<Printer*()> printerFunc( const SearchOptions& opts ) {
    std::function<Printer*()> makePrinter;
    bool colorized = opts.colorized;

    if( opts.piped ) {
        makePrinter = [] {
            PipedPrinter* printer = new PipedPrinter();
            return printer;
        };
    } else {
        makePrinter = [colorized] {
            PrettyPrinter* printer = new PrettyPrinter();
            printer->opts.colorized = colorized;
            return printer;
        };
    }

    return makePrinter;
}

}
