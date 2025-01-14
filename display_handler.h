#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Adafruit_NeoPixel.h>

class DisplayHandler {
    public:
        // Default to pin 14 as per Waveshare docs
        // https://www.waveshare.com/wiki/ESP32-S3-Matrix
        DisplayHandler(uint8_t pin = 14);
        void begin();
        void showRecycling();
        void showRubbish();
        void showNeither();

    private:
        static const uint8_t MATRIX_WIDTH = 8;
        static const uint8_t MATRIX_HEIGHT = 8;

        // We need to keep the brightness low to prevent overheating,
        // see: https://www.waveshare.com/wiki/ESP32-S3-Matrix
        static const uint8_t BRIGHTNESS = 20;

        Adafruit_NeoPixel matrix;
        void fillScreen(uint32_t color);
};

#endif
