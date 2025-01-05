#include "wifi_config.h"

#define PIN_NEOPIXEL 14

void setup() {
    neopixelWrite(PIN_NEOPIXEL, RGB_BRIGHTNESS, 0, 70);
    connectWiFi();
    neopixelWrite(PIN_NEOPIXEL, 0, RGB_BRIGHTNESS, 100);
}

void loop() {
}
