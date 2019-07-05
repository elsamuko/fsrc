#pragma once

#include <string>

namespace fromStd {

// derived from
//! \sa https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/basic_string.tcc#L1199
inline const char* strstr( const char* data, const size_t size, const char* pattern, size_t patternSize ) {

    const char* ptr = data;
    const char* last = data + size;
    size_t length = size;
    const char p0 = pattern[0];

    while( length >= patternSize ) {
        // find first character
        ptr = std::char_traits<char>::find( ptr, length - patternSize + 1, p0 );

        if( !ptr ) { return ptr; }

        // compare with pattern
        if( std::char_traits<char>::compare( ptr, pattern, patternSize ) == 0 ) {
            return ptr;
        }

        length = last - ++ptr;
    }

    return nullptr;
}

}
