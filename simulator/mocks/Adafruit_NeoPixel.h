#pragma once

#include "Arduino.h"

// Mock Adafruit_NeoPixel for simulator
class Adafruit_NeoPixel {
public:
    Adafruit_NeoPixel(uint16_t n, uint8_t pin, uint8_t type = 0)
        : numPixels(n), pin(pin) {}

    void begin() {}
    void show() {}
    void clear() {}
    void setBrightness(uint8_t brightness) { this->brightness = brightness; }

    void setPixelColor(uint16_t n, uint32_t color) {
        if (n < numPixels) {
            // Store color for potential terminal display
        }
    }

    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
        setPixelColor(n, Color(r, g, b));
    }

    uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    uint32_t getPixelColor(uint16_t n) const {
        return 0; // Mock
    }

    void fill(uint32_t color, uint16_t first = 0, uint16_t count = 0) {
        if (count == 0) count = numPixels;
        for (uint16_t i = first; i < first + count && i < numPixels; i++) {
            setPixelColor(i, color);
        }
    }

private:
    uint16_t numPixels;
    uint8_t pin;
    uint8_t brightness = 255;
};

// NeoPixel constants
#define NEO_GRB 0
#define NEO_RGB 1
#define NEO_KHZ800 0
