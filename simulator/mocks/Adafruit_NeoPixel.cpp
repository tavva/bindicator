// ABOUTME: Adafruit_NeoPixel mock implementation for simulator
// ABOUTME: Renders LED matrix using ncurses display manager

#include "Adafruit_NeoPixel.h"
#include "../ncurses_display.h"

void Adafruit_NeoPixel::show() {
    // Render via ncurses display manager
    NcursesDisplay::renderMatrix(pixels, numPixels);
}
