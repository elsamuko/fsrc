#pragma once

#include <iostream>
#include <functional>

#include "boost/timer/timer.hpp"

BOOST_FORCEINLINE long timed1000( const std::string& name, const std::function<void()>& func, const std::function<void()>& cleanup = std::function<void()>() ) {
    boost::timer::cpu_timer stopwatch;
    stopwatch.start();

    for( int i = 0; i < 1000; ++i ) {
        func();
    }

    stopwatch.stop();
    long ns = stopwatch.elapsed().wall;
    printf( "%17s : %6ld us\n", name.c_str(), ns / 1000 );

    // call cleanup function
    if( cleanup ) { cleanup(); }

    return ns;
}
