#pragma once

#include <cstdint>
#include <string>
#include <time.h>
#include "../simulated_time.h"

// Arduino basic types
typedef bool boolean;
typedef uint8_t byte;

// Arduino String class (wraps std::string with Arduino API)
class String : public std::string {
public:
    String() : std::string() {}
    String(const char* str) : std::string(str ? str : "") {}
    String(const std::string& str) : std::string(str) {}
    String(int val) : std::string(std::to_string(val)) {}
    String(unsigned int val) : std::string(std::to_string(val)) {}
    String(long val) : std::string(std::to_string(val)) {}
    String(unsigned long val) : std::string(std::to_string(val)) {}
    String(double val) : std::string(std::to_string(val)) {}

    bool isEmpty() const { return empty(); }
    int toInt() const { return std::stoi(*this); }
    double toDouble() const { return std::stod(*this); }
};

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

// ESP32 time functions
inline bool getLocalTime(struct tm* info, uint32_t ms = 5000) {
    time_t now = time(nullptr);
    localtime_r(&now, info);
    return true;
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
