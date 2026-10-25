#pragma once
// https://mischasan.wordpress.com/2011/07/16/convergence-sse2-and-strstr/

// Copyright (C) 2009-2013 Mischa Sandberg <mischasan@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 2 as
// published by the Free Software Foundation.  You may not use, modify or
// distribute this program under any other version of the GNU General
// Public License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// IF YOU ARE UNABLE TO WORK WITH GPL2, CONTACT ME.
//-------------------------------------------------------------------

#include <cstring>
#include <cstdint>
#ifdef __x86_64__
#include <emmintrin.h>
#else
#include "sse2neon.h"
#endif

#include "winutils.hpp"

#define compxm(a,b) _mm_movemask_epi8(_mm_cmpeq_epi8((a), (b)))
#define xmload(p)   _mm_load_si128((__m128i const *)(p))
#define load16(p)   (*(uint16_t const*)(p))
#define load32(p)   (*(uint32_t const*)(p))

namespace  {

//! \returns c in lower case, only ASCII letters are changed
inline char lowerAscii( const char c ) {
    return ( c >= 'A' && c <= 'Z' ) ? c + ( 'a' - 'A' ) : c;
}

inline char upperAscii( const char c ) {
    return ( c >= 'a' && c <= 'z' ) ? c - ( 'a' - 'A' ) : c;
}

//! \returns true, if the two bytes at tgt equal pat
template<bool ignoreCase>
inline bool isPair( char const* tgt, char const pat[2] ) {
    if constexpr( ignoreCase ) {
        return lowerAscii( tgt[0] ) == pat[0] && lowerAscii( tgt[1] ) == pat[1];
    } else {
        return !memcmp( tgt, pat, 2 );
    }
}

//! \returns first position in [tgt, end - 1), where the two bytes of pat start
//! \param pat lower case, if ignoreCase
//! \note searches until end, NUL bytes in [tgt, end) are content, too
//! \note loads aligned 16 byte blocks, which may reach up to 15 bytes beyond end, but never into the next page
template<bool ignoreCase>
inline char const* scanstr2( char const* tgt, char const* end, char const pat[2] ) {
    // a pair needs two bytes, so the last pair starts before last
    char const* const last = end - 1;

    if( tgt >= last ) { return NULL; }

    __m128i const   p0   = _mm_set1_epi8( pat[0] );
    __m128i const   p1   = _mm_set1_epi8( pat[1] );
    __m128i const   q0   = _mm_set1_epi8( upperAscii( pat[0] ) );
    __m128i const   q1   = _mm_set1_epi8( upperAscii( pat[1] ) );
    unsigned        f    = 15 & ( intptr_t )tgt;

    // bit i is set, if a pair starts at byte i of x (without the pair crossing into the next block)
    auto pairs = [&]( __m128i x ) -> unsigned {
        if constexpr( ignoreCase ) {
            return ( compxm( p0, x ) | compxm( q0, x ) ) & ( ( compxm( p1, x ) | compxm( q1, x ) ) >> 1 );
        } else {
            return compxm( p0, x ) & ( compxm( p1, x ) >> 1 );
        }
    };

    // hits are found in ascending order, so the first hit beyond the end means there is none
    auto checked = [last]( char const * hit ) -> char const* {
        return hit < last ? hit : NULL;
    };

    if( f ) {
        __m128i  x = xmload( tgt - f );
        unsigned v = pairs( x ) >> f;

        if( v ) { return checked( tgt + ffs( v ) - 1 ); }

        tgt += 16 - f;

        if( tgt - 1 >= last ) { return NULL; }

        if( isPair<ignoreCase>( tgt - 1, pat ) ) {
            return tgt - 1;
        }
    }

    while( tgt < last ) {
        __m128i  x = xmload( tgt );
        unsigned v = pairs( x );

        if( v ) { return checked( tgt + ffs( v ) - 1 ); }

        tgt += 16;

        if( tgt - 1 >= last ) { return NULL; }

        if( isPair<ignoreCase>( tgt - 1, pat ) ) {
            return tgt - 1;
        }
    }

    return NULL;
}

}

namespace mischasan {

//! \returns first occurrence of pat in [tgt, tgt + size), or nullptr
inline const char* scanstrN( char const* tgt, const size_t size, char const* pat, const size_t len ) {
    if( len == 1 ) { return static_cast<const char*>( memchr( tgt, pat[0], size ) ); }

    char const* const end = tgt + size;

    for( ; ( tgt = scanstr2<false>( tgt, end, pat ) ); tgt++ ) {
        if( tgt + len > end ) { return nullptr; }

        if( !memcmp( tgt + 2, pat + 2, len - 2 ) ) {
            return tgt;
        }
    }

    return nullptr;
}

//! like scanstrN, but ignores the case of ASCII letters
//! \param pat must be lower case
inline const char* scancasestrN( char const* tgt, const size_t size, char const* pat, const size_t len ) {
    char const* const end = tgt + size;

    if( len == 1 ) {
        for( ; tgt < end; ++tgt ) {
            if( lowerAscii( *tgt ) == pat[0] ) { return tgt; }
        }

        return nullptr;
    }

    for( ; ( tgt = scanstr2<true>( tgt, end, pat ) ); tgt++ ) {
        if( tgt + len > end ) { return nullptr; }

        size_t i = 2;

        while( i < len && lowerAscii( tgt[i] ) == pat[i] ) { ++i; }

        if( i == len ) {
            return tgt;
        }
    }

    return nullptr;
}

}
