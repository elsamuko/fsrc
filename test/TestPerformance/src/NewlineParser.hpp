#pragma once

#include "utils.hpp"

utils::Lines parseContentForLoop( const char* data, const size_t size ) {
    utils::Lines lines;

    if( size == 0 ) { return lines; }

    lines.reserve( 128 );

    char* c_old = ( char* )data;
    char* c_new = c_old;
    char* c_end = c_old + size;

    for( ; *c_new; ++c_new ) {
        // just skip windows line endings
        if( *c_new == '\r' ) {
            ++c_new;
        }

        if( *c_new == '\n' ) {
            lines.emplace_back( c_old, c_new - c_old );
            c_old = c_new + 1;
        }
    }

    if( c_old != c_end ) {
        lines.emplace_back( c_old, c_end - c_old );
    }

    lines.shrink_to_fit();
    return lines;
}

utils::Lines parseContentFind( const char* data, const size_t size ) {
    utils::Lines lines;

    if( size == 0 ) { return lines; }

    lines.reserve( 128 );

    std::string_view view( data, size );
    size_t pos_old = 0;
    size_t pos_new = view.find( '\n' );

    while( pos_new != std::string::npos ) {
        lines.emplace_back( data + pos_old, pos_new - pos_old );
        pos_old = pos_new + 1;
        pos_new = view.find( '\n', pos_old );
    }

    if( pos_old != size ) {
        lines.emplace_back( data + pos_old, size - pos_old );
    }

    lines.shrink_to_fit();
    return lines;
}

utils::Lines parseContentStrchr( const char* data, const size_t size ) {
    utils::Lines lines;

    if( size == 0 ) { return lines; }

    lines.reserve( 128 );

    const char* c_old = data;
    const char* c_new = c_old;
    const char* c_end = c_old + size;

    while( ( c_new = strchr( c_old, '\n' ) ) ) {
        lines.emplace_back( c_old, c_new - c_old );
        c_old = c_new + 1;
    }

    if( c_old != c_end ) {
        lines.emplace_back( c_old, c_end - c_old );
    }

    lines.shrink_to_fit();
    return lines;
}

utils::Lines parseContentMemchr( const char* data, const size_t size ) {
    utils::Lines lines;

    if( size == 0 ) { return lines; }

    lines.reserve( 128 );

    const char* c_old = data;
    const char* c_new = c_old;
    const char* c_end = c_old + size;

    while( ( c_new = static_cast<const char*>( memchr( c_old, '\n', c_end - c_old ) ) ) ) {
        lines.emplace_back( c_old, c_new - c_old );
        c_old = c_new + 1;
    }

    if( c_old != c_end ) {
        lines.emplace_back( c_old, c_end - c_old );
    }

    lines.shrink_to_fit();
    return lines;
}
