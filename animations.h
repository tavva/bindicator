#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>
#include "display_handler.h"
#include "tasks.h"

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
        : r(red), g(green), b(blue) {}
};

class Animations {
    public:
        static void drawError(DisplayHandler& display, Color stroke, Color dot);
        static void drawLoading(DisplayHandler& display, int loadingPos);
        static void drawPulse(DisplayHandler& display, Color color);

    private:
        static const uint8_t exclamation[8][8];
        static int brightnessTick;
        static uint8_t calculateBrightness();
};

#endif
