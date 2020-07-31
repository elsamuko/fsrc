// https://felix.abecassis.me/2011/09/cpp-getting-started-with-sse/
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#
// http://www.tommesani.com/index.php/simd/46-sse-arithmetic.html

#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <emmintrin.h>

#include "ssefind.hpp"

#define LOG( A ) std::cout << A << std::endl;

void findAndLog( const std::string& text, const std::string& pattern ) {
    std::vector<search::Match> matches = sse::find( text.data(), text.size(), pattern.data(), pattern.size() );

    LOG( text );

    for( const search::Match& match : matches ) {
        LOG( std::string( match.first - text.data(), ' ' ) << std::string( pattern.size(), '^' ) );
    }
}

int main( int /*argc*/, char** /*argv*/ ) {

    //                  0123456879abcdef0123456879abcdef0123456879abcdef0123456879abcdef
    std::string text = "This is another text, and now with even longer stuff and two and and so on";
    // findAndLog( text, "oth" );
    // findAndLog( text, "a" );
    findAndLog( text, "stuff" );
    findAndLog( text, "and" );

    return 0;
}
