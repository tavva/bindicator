#include <iostream>
#include <cassert>
#include "simulated_time.h"

void test_millis_basic() {
    SimulatedTime::reset();
    unsigned long t1 = SimulatedTime::millis();
    SimulatedTime::advance(1000);
    unsigned long t2 = SimulatedTime::millis();
    assert(t2 - t1 == 1000);
    std::cout << "✓ test_millis_basic" << std::endl;
}

void test_time_multiplier() {
    SimulatedTime::reset();
    SimulatedTime::setMultiplier(10.0);
    unsigned long t1 = SimulatedTime::millis();
    SimulatedTime::advance(100);  // Real time
    unsigned long t2 = SimulatedTime::millis();
    assert(t2 - t1 == 1000);  // Simulated time (10x)
    std::cout << "✓ test_time_multiplier" << std::endl;
}

void test_millis_overflow() {
    SimulatedTime::reset();
    SimulatedTime::setTime(0xFFFFFF00);
    unsigned long t1 = SimulatedTime::millis();
    SimulatedTime::advance(0x200);
    unsigned long t2 = SimulatedTime::millis();
    // Should wrap around
    assert(t2 < t1);
    assert(t2 == 0x100);
    std::cout << "✓ test_millis_overflow" << std::endl;
}

int main() {
    test_millis_basic();
    test_time_multiplier();
    test_millis_overflow();
    std::cout << "All time tests passed!" << std::endl;
    return 0;
}
