#include "wifi_config.h"
#include "time_manager.h"
#include "oauth_handler.h"
#include "calendar_handler.h"
#include "secrets.h"

#define PIN_NEOPIXEL 14

const String CALENDAR_ID = "primary";

OAuthHandler oauth(CLIENT_ID, CLIENT_SECRET, REFRESH_TOKEN);
CalendarHandler calendar(oauth, CALENDAR_ID);

bool hasRecycling, hasRubbish;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting up...");

    neopixelWrite(PIN_NEOPIXEL, RGB_BRIGHTNESS, 0, 70);
    connectToWiFi();
    setupTime();

    neopixelWrite(PIN_NEOPIXEL, RGB_BRIGHTNESS, 0, 40);
}

void loop() {
    // Check calendar every hour (10 seconds for debugging)
    static unsigned long lastCheck = 0;
    // if (millis() - lastCheck >= 3600000) {
    if (millis() - lastCheck >= 10000) {
        if (calendar.checkForBinEvents(hasRecycling, hasRubbish)) {
            Serial.println("Recycling: " + String(hasRecycling));
            Serial.println("Rubbish: " + String(hasRubbish));
        }
        lastCheck = millis();
    }
}