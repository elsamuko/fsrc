// https://felix.abecassis.me/2011/09/cpp-getting-started-with-sse/
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#
// http://www.tommesani.com/index.php/simd/46-sse-arithmetic.html

#include <cstring>
#include <iostream>
#include <string>
#include <emmintrin.h>

#define LOG( A ) std::cout << A << std::endl;

const char* find( const char* text, size_t s1, const char* term, size_t s2 ) {

    if( s2 == 1 ) {
        return static_cast<const char*>( memchr( static_cast<const void*>( text ), term[0], s1 ) );
    }

    static __m128i lastOne = _mm_setr_epi8( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff );
    const __m128i first  = _mm_set1_epi8( term[0] );
    const __m128i second = _mm_set1_epi8( term[1] );

    size_t stop = s1 - s2 + 1;
    int diff = 0;

    for( size_t i = 0; i < stop; i += 16 ) {
        // load 16 bytes of text into
        const __m128i text16  = _mm_load_si128( ( __m128i const* )( text + i ) );
        // compare text with first char
        const __m128i comp1   = _mm_cmpeq_epi8( text16, first );
        // compare text with 2nd char...
        const __m128i comp2   = _mm_cmpeq_epi8( text16, second );
        // ... and one left
        __m128i shift  = _mm_srli_si128( comp2, 1 );

        // set last byte to ff, if first char from next block is 2nd char
        if( text[i + 16] == term[1] ) {
            shift = _mm_or_si128( shift, lastOne );
        }

        // if both
        const __m128i comp3  = _mm_and_si128( comp1, shift );

        const int mv2        = _mm_movemask_epi8( comp3 );
        diff = ffs( mv2 );

        if( diff && !memcmp( text + i + diff - 1, term, s2 ) ) {
            return text + i + diff - 1;
        }
    }

    return nullptr;
}

void findAndLog( const std::string& text, const std::string& pattern ) {
    const char* pos = find( text.data(), text.size(), pattern.data(), pattern.size() );

    LOG( text );
    LOG( std::string( pos - text.data(), ' ' ) << std::string( pattern.size(), '^' ) );
}

int main( int /*argc*/, char** /*argv*/ ) {

    //                  0123456879abcdef0123456879abcdef0123456879abcdef0123456879abcdef
    std::string text = "This is another text, and now with even longer stuff";
    // findAndLog( text, "oth" );
    // findAndLog( text, "a" );
    findAndLog( text, "stuff" );

    return 0;
}
