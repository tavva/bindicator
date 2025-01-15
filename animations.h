#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>
#include "display_handler.h"
#include "tasks.h"

class Animations {
    public:
        static void drawError(DisplayHandler& display, uint8_t r, uint8_t g, uint8_t b,
                            uint8_t dot_r, uint8_t dot_g, uint8_t dot_b, uint8_t brightness);
        static void drawLoading(DisplayHandler& display, int loadingPos);
        static void drawPulse(DisplayHandler& display, uint8_t r, uint8_t g, uint8_t b,
                            uint8_t brightness);

    private:
        static const uint8_t exclamation[8][8];
};

#endif
