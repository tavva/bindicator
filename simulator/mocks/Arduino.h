#pragma once

#include <cstdint>
#include <string>
#include <time.h>
#include "../simulated_time.h"

// Include FreeRTOS and ESP for ESP32 compatibility
#ifdef SIMULATOR
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ESP.h"
#endif

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

    int indexOf(const char* str) const {
        size_t pos = find(str);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    }

    int indexOf(const String& str) const {
        return indexOf(str.c_str());
    }

    char charAt(size_t index) const {
        if (index < length()) {
            return (*this)[index];
        }
        return '\0';
    }

    void trim() {
        // Remove leading whitespace
        size_t start = 0;
        while (start < length() && isspace((*this)[start])) {
            start++;
        }
        // Remove trailing whitespace
        size_t end = length();
        while (end > start && isspace((*this)[end - 1])) {
            end--;
        }
        *this = substr(start, end - start);
    }
};

// Arduino constants
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define PI 3.1415926535897932384626433832795

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

inline void configTime(long gmtOffset_sec, int daylightOffset_sec, const char* server1, const char* server2 = nullptr, const char* server3 = nullptr) {
    // Mock NTP time configuration
    // In simulator, we just use system time
}

// Pin functions (no-op for simulator)
inline void pinMode(uint8_t pin, uint8_t mode) {}
inline int digitalRead(uint8_t pin) { return LOW; }
inline void digitalWrite(uint8_t pin, uint8_t val) {}

// Forward declare IPAddress for Serial
class IPAddress;

// Serial class
class SerialClass {
public:
    void begin(unsigned long baud) {}
    void println(const char* str);
    void println(const String& str);
    void println(const IPAddress& ip);
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
