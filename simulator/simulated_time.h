#pragma once
#include <cstdint>
#include <mutex>
#include <chrono>

class SimulatedTime {
public:
    static uint32_t millis();
    static void advance(uint32_t ms);
    static void setTime(uint32_t ms);
    static void setMultiplier(float multiplier);
    static void reset();

private:
    static uint32_t baseTime;
    static float timeMultiplier;
    static std::mutex timeMutex;
    static std::chrono::steady_clock::time_point startTime;
};
