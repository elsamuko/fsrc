#pragma once

#include <cstring>
#include <vector>
#ifdef __x86_64__
#include <emmintrin.h>
#else
#include "sse2neon.h"
#endif

#include "types.hpp"
#ifdef _WIN32
#include "winutils.hpp"
#endif

#define SSE128 16

namespace sse {
inline std::vector<search::Match> find( const std::string_view& text, const std::string& term ) {
    std::vector<search::Match> matches;
    const char* start = text.data();

    if( term.size() == 1 ) {
        const char* pos = text.data();

        while( ( pos = static_cast<const char*>( memchr( static_cast<const void*>( pos ),
                                                         term[0],
                                                         text.size() - ( pos - start ) ) ) ) ) {
            auto iter = text.cbegin() + ( pos - start );
            matches.emplace_back( iter, iter + 1 );
            ++pos;
        }

        return matches;
    }

    static const __m128i lastOne = _mm_setr_epi8( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff );
    const __m128i first  = _mm_set1_epi8( term[0] );
    const __m128i second = _mm_set1_epi8( term[1] );

    size_t blocks = ( text.size() - term.size() ) / SSE128 + 1;
    int diff = 0;

    for( size_t block = 0; block < blocks; ++block ) {
        // load 16 bytes of text
        const __m128i text16  = _mm_load_si128( ( __m128i const* )( start + block * SSE128 ) );
        // compare text with first char
        const __m128i comp1   = _mm_cmpeq_epi8( text16, first );
        // compare text with 2nd char...
        const __m128i comp2   = _mm_cmpeq_epi8( text16, second );
        // ... and one left
        __m128i shift  = _mm_srli_si128( comp2, 1 );

        // set last byte to ff, checking is faster than branching
        shift = _mm_or_si128( shift, lastOne );

        // if both have hits
        const __m128i comp3  = _mm_and_si128( comp1, shift );

        // get its position
        int mv2 = _mm_movemask_epi8( comp3 );

        while( ( diff = ffs( mv2 ) ) ) {
            const char* pos = start + block * SSE128 + diff - 1;

            if( !memcmp( pos + 2, &term[2], term.size() - 2 ) ) {
                auto iter = text.cbegin() + ( pos - start );
                matches.emplace_back( iter, iter + term.size() );
            }

            mv2 ^= ( 1 << ( diff - 1 ) );
        }
    }

    return matches;
}
}
