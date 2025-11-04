#pragma once

class SimulatedTime {
public:
    static unsigned long millis();
    static void advance(unsigned long ms);
    static void setTime(unsigned long ms);
    static void setMultiplier(float multiplier);
    static void reset();

private:
    static unsigned long baseTime;
    static float timeMultiplier;
};
