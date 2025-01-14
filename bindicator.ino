#include "wifi_config.h"
#include "time_manager.h"
#include "oauth_handler.h"
#include "calendar_handler.h"
#include "secrets.h"
#include "display_handler.h"

#define PIN_NEOPIXEL 14

OAuthHandler oauth(CLIENT_ID, CLIENT_SECRET, REFRESH_TOKEN);
CalendarHandler calendar(oauth, CALENDAR_ID);
DisplayHandler display;
bool hasRecycling, hasRubbish;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting up...");

    display.begin();
    display.showNeither();

    connectToWiFi();
    setupTime();

    if (calendar.checkForBinEvents(hasRecycling, hasRubbish)) {
        updateDisplay(hasRecycling, hasRubbish);
    }

    Serial.println("Setup complete - entering loop");

    // Ensure all serial data is sent to prevent hangs
    Serial.flush();
    delay(100);
}

void loop() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck >= 3600000) {
        Serial.println("Checking for bin events");
        if (calendar.checkForBinEvents(hasRecycling, hasRubbish)) {
            updateDisplay(hasRecycling, hasRubbish);
        }
        lastCheck = millis();
    }

    display.update();
    yield(); // to prevent hangs
}

void updateDisplay(bool hasRecycling, bool hasRubbish) {
    if (hasRecycling) {
        display.showRecycling();
    } else if (hasRubbish) {
        display.showRubbish();
    } else {
        display.showNeither();
    }
}
