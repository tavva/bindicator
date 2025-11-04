// ABOUTME: Simulated time system for testing time-dependent firmware behaviour
// ABOUTME: Supports time acceleration, manual advancement, and overflow testing

#include "simulated_time.h"
#include <cstdint>

unsigned long SimulatedTime::baseTime = 0;
float SimulatedTime::timeMultiplier = 1.0f;

unsigned long SimulatedTime::millis() {
    return baseTime;
}

void SimulatedTime::advance(unsigned long ms) {
    unsigned long delta = static_cast<unsigned long>(ms * timeMultiplier);
    baseTime = static_cast<uint32_t>(baseTime + delta);
}

void SimulatedTime::setTime(unsigned long ms) {
    baseTime = ms;
}

void SimulatedTime::setMultiplier(float multiplier) {
    timeMultiplier = multiplier;
}

void SimulatedTime::reset() {
    baseTime = 0;
    timeMultiplier = 1.0f;
}
