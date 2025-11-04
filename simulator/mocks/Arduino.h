#pragma once

#include <cstdint>
#include <string>
#include "../simulated_time.h"

// Arduino basic types
typedef bool boolean;
typedef uint8_t byte;
typedef std::string String;

// Arduino constants
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Time functions
inline unsigned long millis() {
    return SimulatedTime::millis();
}

inline void delay(unsigned long ms) {
    SimulatedTime::advance(ms);
}

// Pin functions (no-op for simulator)
inline void pinMode(uint8_t pin, uint8_t mode) {}
inline int digitalRead(uint8_t pin) { return LOW; }
inline void digitalWrite(uint8_t pin, uint8_t val) {}

// Serial class
class SerialClass {
public:
    void begin(unsigned long baud) {}
    void println(const char* str);
    void println(const String& str);
    void println(int value);
    void println(unsigned long value);
    void println(double value);
    void print(const char* str);
    void print(const String& str);
    void print(int value);
    void print(unsigned long value);
    void print(double value);
    void printf(const char* format, ...);

    bool available() { return false; }
    String readStringUntil(char terminator) { return ""; }
};

extern SerialClass Serial;
