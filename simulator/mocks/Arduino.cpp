// ABOUTME: Arduino core API mocks for desktop simulator
// ABOUTME: Provides Serial output, pin operations, and basic Arduino functions

#include "Arduino.h"
#include <iostream>
#include <cstdarg>
#include <cstdio>

SerialClass Serial;

void SerialClass::println(const char* str) {
    std::cout << str << std::endl;
}

void SerialClass::println(const String& str) {
    std::cout << str << std::endl;
}

void SerialClass::print(const char* str) {
    std::cout << str;
}

void SerialClass::print(const String& str) {
    std::cout << str;
}

void SerialClass::printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
