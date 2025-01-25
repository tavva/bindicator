#ifndef ARDUINO_H
#define ARDUINO_H

#include <string>
#include <iostream>
#include <stdarg.h>
#include "time_mock.h"

// String class
class String {
public:
    String() : str("") {}
    String(const char* s) : str(s ? s : "") {}
    String(const String& s) : str(s.str) {}
    String(char c) : str(1, c) {}

    bool isEmpty() const { return str.empty(); }
    const char* c_str() const { return str.c_str(); }
    unsigned int length() const { return str.length(); }

    char charAt(unsigned int index) const {
        if (index >= str.length()) return 0;
        return str[index];
    }

    // Assignment operators
    String& operator+=(const String& rhs) {
        str += rhs.str;
        return *this;
    }

    String& operator+=(const char* rhs) {
        str += (rhs ? rhs : "");
        return *this;
    }

    String& operator+=(char c) {
        str += c;
        return *this;
    }

    // Comparison operators
    bool operator==(const String& other) const { return str == other.str; }
    bool operator==(const char* other) const { return str == (other ? other : ""); }
    bool operator!=(const String& other) const { return !(*this == other); }
    bool operator!=(const char* other) const { return !(*this == other); }

private:
    std::string str;
};

// Non-member operators for symmetrical comparison
inline bool operator==(const char* lhs, const String& rhs) {
    return rhs == lhs;
}

inline bool operator!=(const char* lhs, const String& rhs) {
    return rhs != lhs;
}

// Then Serial class with output control
class SerialClass {
public:
    SerialClass() : suppress_output(false) {}

    void suppressOutput(bool suppress) { suppress_output = suppress; }

    void print(const char* message) {
        if (!suppress_output) std::cout << message;
    }

    void print(const String& message) {
        if (!suppress_output) std::cout << message.c_str();
    }

    void println(const char* message) {
        if (!suppress_output) std::cout << message << std::endl;
    }

    void println(const String& message) {
        if (!suppress_output) std::cout << message.c_str() << std::endl;
    }

    void printf(const char* format, ...) {
        if (!suppress_output) {
            va_list args;
            va_start(args, format);
            vprintf(format, args);
            va_end(args);
        }
    }

private:
    bool suppress_output;
};

extern SerialClass Serial;

#endif
