// ABOUTME: Arduino core API mocks for desktop simulator
// ABOUTME: Provides Serial output, pin operations, and basic Arduino functions

#include "Arduino.h"
#include "WiFi.h"
#include "../ncurses_display.h"
#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <sstream>

SerialClass Serial;

void SerialClass::println(const char* str) {
    std::string output = std::string(str) + "\n";
    NcursesDisplay::printConsole(output.c_str());
}

void SerialClass::println(const String& str) {
    std::string output = std::string(str) + "\n";
    NcursesDisplay::printConsole(output.c_str());
}

void SerialClass::println(const IPAddress& ip) {
    std::string output = ip.toString() + "\n";
    NcursesDisplay::printConsole(output.c_str());
}

void SerialClass::println(int value) {
    std::ostringstream oss;
    oss << value << "\n";
    NcursesDisplay::printConsole(oss.str().c_str());
}

void SerialClass::println(unsigned long value) {
    std::ostringstream oss;
    oss << value << "\n";
    NcursesDisplay::printConsole(oss.str().c_str());
}

void SerialClass::println(double value) {
    std::ostringstream oss;
    oss << value << "\n";
    NcursesDisplay::printConsole(oss.str().c_str());
}

void SerialClass::print(const char* str) {
    NcursesDisplay::printConsole(str);
}

void SerialClass::print(const String& str) {
    NcursesDisplay::printConsole(str.c_str());
}

void SerialClass::print(int value) {
    std::ostringstream oss;
    oss << value;
    NcursesDisplay::printConsole(oss.str().c_str());
}

void SerialClass::print(unsigned long value) {
    std::ostringstream oss;
    oss << value;
    NcursesDisplay::printConsole(oss.str().c_str());
}

void SerialClass::print(double value) {
    std::ostringstream oss;
    oss << value;
    NcursesDisplay::printConsole(oss.str().c_str());
}

void SerialClass::printf(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    NcursesDisplay::printConsole(buffer);
}

bool SerialClass::available() {
    return NcursesDisplay::hasSerialCommand();
}

String SerialClass::readStringUntil(char terminator) {
    return String(NcursesDisplay::getSerialCommand());
}

int digitalRead(uint8_t pin) {
    // Simulate button press (active LOW with INPUT_PULLUP)
    static unsigned long pressStartTime = 0;
    static bool wasPressed = false;

    bool currentlyPressed = NcursesDisplay::isButtonPressed();

    // Detect button press start
    if (currentlyPressed && !wasPressed) {
        pressStartTime = millis();
        wasPressed = true;
    }

    // Check for auto-release after duration
    if (currentlyPressed && wasPressed) {
        unsigned long elapsed = millis() - pressStartTime;
        unsigned long duration = NcursesDisplay::getButtonPressDuration();

        // Auto-release after the specified duration
        if (elapsed >= duration) {
            NcursesDisplay::releaseButton();
            wasPressed = false;
            return HIGH;
        }
        return LOW;
    }

    // Button not pressed or was released
    if (!currentlyPressed) {
        wasPressed = false;
    }

    return HIGH;
}
