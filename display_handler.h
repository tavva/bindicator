#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Adafruit_NeoPixel.h>

class DisplayHandler {
    public:
        // Default to pin 14 as per Waveshare docs
        // https://www.waveshare.com/wiki/ESP32-S3-Matrix
        DisplayHandler(uint8_t pin = 14);
        void begin();
        void update();
        void setPixelColor(uint16_t pixel, uint32_t color);

        Adafruit_NeoPixel matrix;

    private:
        static const uint8_t MATRIX_WIDTH = 8;
        static const uint8_t MATRIX_HEIGHT = 8;

        static const uint8_t MAX_BRIGHTNESS = 20;
        static const uint8_t MIN_BRIGHTNESS = 0;

        // We need to keep the brightness low to prevent overheating,
        // see: https://www.waveshare.com/wiki/ESP32-S3-Matrix
        static const uint8_t BRIGHTNESS = MAX_BRIGHTNESS;

        void fillScreen(uint32_t color);

        uint32_t currentColor;
        bool isPulsing;
        unsigned long lastPulseUpdate;
        uint8_t pulseValue;
        bool pulseIncreasing;

        uint16_t getRotatedPixel(uint16_t pixel);
};

#endif
