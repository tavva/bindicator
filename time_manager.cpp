#include "time_manager.h"
#include <time.h>

// NTP Server details
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;      // GMT+0
const int   daylightOffset_sec = 3600; // +1 hour for BST

bool setupTime() {
    Serial.println("Setting up time sync...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return false;
    }

    Serial.println("Time set from NTP");
    return true;
}
