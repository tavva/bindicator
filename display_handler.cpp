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
    matrix.show();
}

void DisplayHandler::showRecycling() {
    currentColor = matrix.Color(0, 50, 0);  // Green
    fillScreen(currentColor);
    isPulsing = true;
}

void DisplayHandler::showRubbish() {
    currentColor = matrix.Color(20, 40, 0);  // Brown
    fillScreen(currentColor);
    isPulsing = true;
}

void DisplayHandler::showNeither() {
    currentColor = matrix.Color(0, 0, 50);  // Blue
    fillScreen(currentColor);
    isPulsing = true;
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
