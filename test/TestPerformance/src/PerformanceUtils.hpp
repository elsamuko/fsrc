#pragma once

#include <iostream>
#include <functional>

#include "stopwatch.hpp"

inline long timed1000( const std::string& name, const std::function<void()>& func, const std::function<void()>& cleanup = std::function<void()>() ) {
    long ns = 0;
    STOPWATCH
    START

    for( int i = 0; i < 1000; ++i ) {
        func();
    }

    STOP( ns );
    printf( "%17s : %6ld us\n", name.c_str(), ns / 1000 );

    // call cleanup function
    if( cleanup ) { cleanup(); }

    return ns;
}
