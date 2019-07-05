#pragma once

#include <chrono>

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
