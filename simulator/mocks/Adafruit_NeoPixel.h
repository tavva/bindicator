#pragma once

#include "Arduino.h"

// Mock Adafruit_NeoPixel for simulator
class Adafruit_NeoPixel {
private:
    uint16_t numPixels;
    uint8_t pin;
    uint8_t brightness = 255;
    uint32_t* pixels;

public:
    Adafruit_NeoPixel(uint16_t n, uint8_t pin, uint8_t type = 0)
        : numPixels(n), pin(pin) {
        pixels = new uint32_t[n];
        clear();
    }

    ~Adafruit_NeoPixel() {
        delete[] pixels;
    }

    void begin() {}

    void show();  // Implemented in .cpp file

    void clear() {
        for (uint16_t i = 0; i < numPixels; i++) {
            pixels[i] = 0;
        }
    }
    void setBrightness(uint8_t brightness) { this->brightness = brightness; }

    void setPixelColor(uint16_t n, uint32_t color) {
        if (n < numPixels) {
            pixels[n] = color;
        }
    }

    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
        setPixelColor(n, Color(r, g, b));
    }

    uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    uint32_t getPixelColor(uint16_t n) const {
        if (n < numPixels) {
            return pixels[n];
        }
        return 0;
    }

    void fill(uint32_t color, uint16_t first = 0, uint16_t count = 0) {
        if (count == 0) count = numPixels;
        for (uint16_t i = first; i < first + count && i < numPixels; i++) {
            setPixelColor(i, color);
        }
    }

};

// NeoPixel constants
#define NEO_GRB 0
#define NEO_RGB 1
#define NEO_KHZ800 0
