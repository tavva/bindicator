// ABOUTME: Simulated time system for testing time-dependent firmware behaviour
// ABOUTME: Supports time acceleration, manual advancement, and overflow testing

#include "simulated_time.h"
#include <cstdint>
#include <mutex>
#include <chrono>

uint32_t SimulatedTime::baseTime = 0;
float SimulatedTime::timeMultiplier = 1.0f;
std::mutex SimulatedTime::timeMutex;
std::chrono::steady_clock::time_point SimulatedTime::startTime = std::chrono::steady_clock::now();

uint32_t SimulatedTime::millis() {
    std::lock_guard<std::mutex> lock(timeMutex);

    // Calculate real elapsed time since start
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

    // Apply time multiplier and add to base time
    return baseTime + static_cast<uint32_t>(elapsed * timeMultiplier);
}

void SimulatedTime::advance(uint32_t ms) {
    std::lock_guard<std::mutex> lock(timeMutex);
    uint32_t delta = static_cast<uint32_t>(ms * timeMultiplier);
    baseTime = baseTime + delta;
}

void SimulatedTime::setTime(uint32_t ms) {
    std::lock_guard<std::mutex> lock(timeMutex);
    baseTime = ms;
    startTime = std::chrono::steady_clock::now();
}

void SimulatedTime::setMultiplier(float multiplier) {
    std::lock_guard<std::mutex> lock(timeMutex);
    timeMultiplier = multiplier;
}

void SimulatedTime::reset() {
    std::lock_guard<std::mutex> lock(timeMutex);
    baseTime = 0;
    timeMultiplier = 1.0f;
    startTime = std::chrono::steady_clock::now();
}
