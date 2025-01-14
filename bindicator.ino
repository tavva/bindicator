#include "wifi_config.h"
#include "time_manager.h"
#include "secrets.h"

#define PIN_NEOPIXEL 14

void setup() {
    neopixelWrite(PIN_NEOPIXEL, RGB_BRIGHTNESS, 0, 70);
    connectToWiFi();
    setupTime();

    neopixelWrite(PIN_NEOPIXEL, RGB_BRIGHTNESS, 0, 40);
}

void loop() {
}
