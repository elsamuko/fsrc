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

inline char const* scanstr2( char const* tgt, char const pat[2] ) {
    __m128i const   zero = _mm_setzero_si128();
    __m128i const   p0   = _mm_set1_epi8( pat[0] );
    __m128i const   p1   = _mm_set1_epi8( pat[1] );
    unsigned        f    = 15 & ( intptr_t )tgt;
    uint16_t        pair = load16( pat );

    if( f ) {
        __m128i  x = xmload( tgt - f );
        unsigned u = compxm( zero, x ) >> f;
        unsigned v = ( ( compxm( p0, x ) & ( compxm( p1, x ) >> 1 ) ) >> f ) & ~u & ( u - 1 );

        if( v ) { return tgt + ffs( v ) - 1; }

        if( u ) { return  NULL; }

        tgt += 16 - f;

        if( load16( tgt - 1 ) == pair ) {
            return tgt - 1;
        }
    }

    while( 1 ) {
        __m128i  x = xmload( tgt );
        unsigned u = compxm( zero, x );
        unsigned v = compxm( p0, x ) & ( compxm( p1, x ) >> 1 ) & ~u & ( u - 1 );

        if( v ) { return tgt + ffs( v ) - 1; }

        if( u ) { return  NULL; }

        tgt += 16;

        if( load16( tgt - 1 ) == pair ) {
            return tgt - 1;
        }
    }
}

}

namespace mischasan {

inline const char* scanstrN( char const* tgt, const int size, char const* pat, int len ) {
    if( len == 1 ) { return static_cast<const char*>( memchr( tgt, pat[0], size ) ); }

    for( ; ( tgt = scanstr2( tgt, pat ) ); tgt++ ) {
        if( !memcmp( tgt + 2, pat + 2, len - 2 ) ) {
            return tgt;
        }
    }

    return nullptr;
}

}
