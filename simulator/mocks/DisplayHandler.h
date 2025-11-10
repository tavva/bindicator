#pragma once

#include "Arduino.h"
#include <cstdint>

// Mock LED matrix class with terminal rendering
class Adafruit_NeoMatrix {
private:
    static const int WIDTH = 8;
    static const int HEIGHT = 8;
    uint32_t pixels[WIDTH * HEIGHT];

public:
    Adafruit_NeoMatrix() {
        clear();
    }

    void begin() {}

    void clear() {
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            pixels[i] = 0;
        }
    }

    void show();

    void setPixelColor(uint16_t n, uint32_t c) {
        if (n < WIDTH * HEIGHT) {
            pixels[n] = c;
        }
    }

    void setPixelColor(uint16_t x, uint16_t y, uint32_t c) {
        if (x < WIDTH && y < HEIGHT) {
            pixels[y * WIDTH + x] = c;
        }
    }

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
