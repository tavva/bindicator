// ABOUTME: Simulated time system for testing time-dependent firmware behaviour
// ABOUTME: Supports time acceleration, manual advancement, and overflow testing

#include "simulated_time.h"
#include <cstdint>
#include <mutex>

uint32_t SimulatedTime::baseTime = 0;
float SimulatedTime::timeMultiplier = 1.0f;
std::mutex SimulatedTime::timeMutex;

uint32_t SimulatedTime::millis() {
    std::lock_guard<std::mutex> lock(timeMutex);
    return baseTime;
}

void SimulatedTime::advance(uint32_t ms) {
    std::lock_guard<std::mutex> lock(timeMutex);
    uint32_t delta = static_cast<uint32_t>(ms * timeMultiplier);
    baseTime = baseTime + delta;
}

void SimulatedTime::setTime(uint32_t ms) {
    std::lock_guard<std::mutex> lock(timeMutex);
    baseTime = ms;
}

void SimulatedTime::setMultiplier(float multiplier) {
    std::lock_guard<std::mutex> lock(timeMutex);
    timeMultiplier = multiplier;
}

void SimulatedTime::reset() {
    std::lock_guard<std::mutex> lock(timeMutex);
    baseTime = 0;
    timeMultiplier = 1.0f;
}
