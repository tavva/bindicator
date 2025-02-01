#include "display_handler.h"

DisplayHandler::DisplayHandler(uint8_t pin)
    : matrix(MATRIX_WIDTH * MATRIX_HEIGHT, pin, NEO_RGB + NEO_KHZ800),
      isPulsing(false),
      lastPulseUpdate(0),
      pulseValue(MIN_BRIGHTNESS),
      pulseIncreasing(true) {
}

void DisplayHandler::begin() {
    matrix.begin();
    matrix.setBrightness(BRIGHTNESS);
    matrix.clear();

    for(int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
        matrix.setPixelColor(i, 0);
    }
    matrix.show();
    delay(50);
}

// Add this helper method to rotate pixel coordinates 180 degrees
uint16_t DisplayHandler::getRotatedPixel(uint16_t pixel) {
    // For 8x8 matrix, rotate 180 degrees means:
    // (x,y) becomes (7-x, 7-y)
    uint8_t x = pixel % MATRIX_WIDTH;
    uint8_t y = pixel / MATRIX_WIDTH;
    return ((MATRIX_HEIGHT - 1 - y) * MATRIX_WIDTH) + (MATRIX_WIDTH - 1 - x);
}

void DisplayHandler::setPixelColor(uint16_t pixel, uint32_t color) {
    matrix.setPixelColor(getRotatedPixel(pixel), color);
}

void DisplayHandler::update() {
    if (!isPulsing) return;

    unsigned long currentMillis = millis();
    if (currentMillis - lastPulseUpdate < 30) return;

    lastPulseUpdate = currentMillis;

    float t = (float)(currentMillis) / 40000.0;
    float sine = sin(2 * PI * t);
    float normalized = (sine + 1.0) / 2.0;

    uint8_t r = ((currentColor >> 16) & 0xFF) * normalized;
    uint8_t g = ((currentColor >> 8) & 0xFF) * normalized;
    uint8_t b = (currentColor & 0xFF) * normalized;

    uint32_t scaledColor = matrix.Color(r, g, b);
    fillScreen(scaledColor);
}

void DisplayHandler::fillScreen(uint32_t color) {
    for(int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
        matrix.setPixelColor(i, color);
    }
    matrix.show();
}
