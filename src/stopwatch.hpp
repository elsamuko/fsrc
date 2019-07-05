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
        using clock = std::chrono::high_resolution_clock;
        using ns_type = std::chrono::nanoseconds::rep;
    public:
        //! starts stopwatch
        inline void start() {
            started = clock::now();
        }
        //! stops stopwatch and returns elapsed nanoseconds
        inline ns_type stop() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>( clock::now() - started ).count();
        }
    private:
        std::chrono::time_point<clock> started;
};
