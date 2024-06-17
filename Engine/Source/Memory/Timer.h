#pragma once
#include <chrono>
#include <iostream>

#define TIMER(DebugName) STimer Timer(##DebugName)

class STimer
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    STimer(const char* InDebugName)
        : StartTime(Clock::now()), DebugName(InDebugName)
    {
    }

    ~STimer()
    {
        auto EndTime = Clock::now();
        auto Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(EndTime - StartTime).count();
        std::cout << "(" << DebugName << ") Elapsed time: " << Duration << " ns" << std::endl;
    }
    

private:
    TimePoint StartTime;

    const char* DebugName;
};
