#pragma once

#include <cstring>
#include <vector>
#include <emmintrin.h>

namespace sse {
inline std::vector<const char*> find( const char* text, size_t s1, const char* term, size_t s2 ) {

    std::vector<const char*> matches;

    if( s2 == 1 ) {
        const char* pos = text;

        while( ( pos = static_cast<const char*>( memchr( static_cast<const void*>( pos ), term[0], s1 - ( pos - text ) ) ) ) ) {
            matches.push_back( pos );
            ++pos;
        }

        return matches;
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

        int mv2 = _mm_movemask_epi8( comp3 );
        diff = ffs( mv2 );

        while( ( diff = ffs( mv2 ) ) ) {
            if( !memcmp( text + i + diff - 1, term, s2 ) ) {
                matches.push_back( text + i + diff - 1 );
            }

            mv2 ^= ( 1 << ( diff - 1 ) );
        }
    }

    return matches;
}
}
