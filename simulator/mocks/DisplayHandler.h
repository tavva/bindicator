#pragma once

#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include <cstdint>

class DisplayHandler {
private:
    static const uint8_t MATRIX_WIDTH = 8;
    static const uint8_t MATRIX_HEIGHT = 8;

    // Helper method to rotate pixel coordinates 180 degrees
    uint16_t getRotatedPixel(uint16_t pixel) {
        uint8_t x = pixel % MATRIX_WIDTH;
        uint8_t y = pixel / MATRIX_WIDTH;
        return ((MATRIX_HEIGHT - 1 - y) * MATRIX_WIDTH) + (MATRIX_WIDTH - 1 - x);
    }

public:
    Adafruit_NeoPixel matrix;

    DisplayHandler(uint8_t pin = 14) : matrix(MATRIX_WIDTH * MATRIX_HEIGHT, pin, NEO_RGB) {}

    void begin() {
        Serial.println("Display initialized");
        matrix.begin();
        matrix.clear();
        matrix.show();
    }

    void setPixelColor(uint16_t pixel, uint32_t color) {
        matrix.setPixelColor(getRotatedPixel(pixel), color);
    }
};
