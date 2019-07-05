#pragma once

#include <chrono>

#if DETAILED_STATS
#define STOPWATCH static thread_local StopWatch stopwatch;
#define START stopwatch.start();
#define STOP( T ) T += stopwatch.stop();
#else
#define STOPWATCH
#define START
#define STOP( T )
#endif

class StopWatch {
    public:
        //! starts stopwatch
        inline void start() {
            started = std::chrono::high_resolution_clock::now();
        }
        //! stops stopwatch and returns elapsed nanoseconds
        inline long stop() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now() - started ).count();
        }
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> started;
};
