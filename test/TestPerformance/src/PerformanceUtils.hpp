#pragma once

#include <iostream>
#include <functional>

#include "boost/timer/timer.hpp"

BOOST_FORCEINLINE boost::int_least64_t timed1000( const std::string& name, const std::function<void()>& func ) {
    boost::timer::cpu_timer stopwatch;
    stopwatch.start();

    for( int i = 0; i < 1000; ++i ) {
        func();
    }

    stopwatch.stop();
    boost::int_least64_t ns = stopwatch.elapsed().wall;
    printf( "%17s : %6lld ms\n", name.c_str(), ns / 1000 );
    return ns;
}
