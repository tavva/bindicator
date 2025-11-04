#pragma once
#include <cstdint>
#include <mutex>

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
};
