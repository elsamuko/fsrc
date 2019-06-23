#pragma once

utils::Lines parseContentForLoop( const char* data, const size_t size ) {
    utils::Lines lines;
    lines.reserve( 128 );

    if( size == 0 ) { return lines; }

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
    lines.reserve( 128 );

    if( size == 0 ) { return lines; }

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
