#include "display_handler.h"

DisplayHandler::DisplayHandler(uint8_t pin)
    : matrix(MATRIX_WIDTH * MATRIX_HEIGHT, pin, NEO_GRB + NEO_KHZ800) {
}

void DisplayHandler::begin() {
    matrix.begin();
    matrix.setBrightness(BRIGHTNESS);
    matrix.clear();
    matrix.show();
}

void DisplayHandler::showRecycling() {
    Serial.println("Showing recycling");
    fillScreen(matrix.Color(0, 50, 0));  // Green
}

void DisplayHandler::showRubbish() {
    Serial.println("Showing rubbish");
    fillScreen(matrix.Color(40, 20, 0));  // Brown
}

void DisplayHandler::showNeither() {
    Serial.println("Showing neither");
    fillScreen(matrix.Color(5, 5, 5));  // Faint white
}

void DisplayHandler::fillScreen(uint32_t color) {
    for(int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
        matrix.setPixelColor(i, color);
    }
    matrix.show();
}
