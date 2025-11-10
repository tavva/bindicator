#pragma once

#include "Arduino.h"

// Mock LED matrix class
class Adafruit_NeoMatrix {
public:
    void begin() {}
    void clear() {}
    void show() {}
    void setPixelColor(uint16_t n, uint32_t c) {}
    void setPixelColor(uint16_t x, uint16_t y, uint32_t c) {}
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
};

class DisplayHandler {
public:
    Adafruit_NeoMatrix matrix;

    void begin() {
        Serial.println("Display initialized");
    }
};
