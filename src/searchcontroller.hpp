#pragma once

#include <mutex>
#include <atomic>

#include "utils.hpp"
#include "types.hpp"
#include "stopwatch.hpp"
#include "searchoptions.hpp"

struct Printer;
struct Searcher;

struct Stats {
    std::atomic_size_t matches = {0};
    std::atomic_size_t filesSearched = {0};
    std::atomic_size_t filesMatched = {0};
    std::atomic_size_t bytesRead = {0};

    std::atomic_llong t_recurse = {0}; // time to recurse directory
    std::atomic_llong t_read = {0};    // time to read files
    std::atomic_llong t_search = {0};  // time to search in files
    std::atomic_llong t_collect = {0}; // time to prepare matches for printing
    std::atomic_llong t_print = {0};   // time to print
};

struct SearchController {
    std::mutex m;
    std::string term;
    SearchOptions opts;
    std::function<Searcher*()> makeSearcher;
    std::function<Printer*()> makePrinter;
    Stats stats;
    Color gray = Color::Gray;

    SearchController( const SearchOptions& opts, std::function<Searcher*()> searcher, std::function<Printer*()> printer ):
        opts( opts ),
        makeSearcher( searcher ),
        makePrinter( printer ) {

        term = opts.term;

        if( !opts.colorized ) {
            gray = Color::Neutral;
        }
    }

    ~SearchController() {}

    void onAllFiles();
    void onGitFiles();

    void printHeader();
    void printGitHeader();
    void printStats();
    void printFooter( const StopWatch::ns_type& ms );

    void search( const sys_string& path );
};
