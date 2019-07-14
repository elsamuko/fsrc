#pragma once

#include "prettyprinter.hpp"
#include "pipedprinter.hpp"
#include "htmlprinter.hpp"
#include "searchoptions.hpp"

namespace printerfactory {

std::function<Printer*()> printerFunc( const SearchOptions& opts ) {
    if( opts.html ) {
        return [opts] {
            HtmlPrinter* printer = new HtmlPrinter( opts );
            return printer;
        };
    }

    if( opts.piped ) {
        return [opts] {
            PipedPrinter* printer = new PipedPrinter( opts );
            return printer;
        };
    } else {
        return [opts] {
            PrettyPrinter* printer = new PrettyPrinter( opts );
            return printer;
        };
    }
}

}
