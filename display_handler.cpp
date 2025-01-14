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
    matrix.setBrightness(MAX_BRIGHTNESS);
    matrix.clear();
    matrix.show();
}

void DisplayHandler::showRecycling() {
    currentColor = matrix.Color(0, 50, 0);  // Green
    isPulsing = true;
    fillScreen(currentColor);
}

void DisplayHandler::showRubbish() {
    currentColor = matrix.Color(40, 20, 0);  // Brown
    isPulsing = true;
    fillScreen(currentColor);
}

void DisplayHandler::showNeither() {
    currentColor = matrix.Color(5, 5, 5);  // Faint white
    isPulsing = false;
    matrix.setBrightness(MIN_BRIGHTNESS);
    fillScreen(currentColor);
}

void DisplayHandler::update() {
    if (!isPulsing) return;

    unsigned long currentMillis = millis();
    if (currentMillis - lastPulseUpdate >= 20) {
        lastPulseUpdate = currentMillis;

        float t = (float)(millis()) / 2000.0; // total cycle time
        float sine = sin(t * PI);
        float normalized = (sine + 1.0) / 2.0;

        pulseValue = MIN_BRIGHTNESS + (normalized * (MAX_BRIGHTNESS - MIN_BRIGHTNESS));

        matrix.setBrightness(static_cast<uint8_t>(pulseValue));
        fillScreen(currentColor);
        yield();
    }
}

void DisplayHandler::fillScreen(uint32_t color) {
    for(int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
        matrix.setPixelColor(i, color);
    }
    matrix.show();
}
