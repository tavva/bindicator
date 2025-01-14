#include "time_manager.h"
#include <time.h>

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;      // GMT+0
const int   daylightOffset_sec = 3600; // +1 hour for BST

bool setupTime() {
    Serial.println("Setting up time sync...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time from NTP");
        return false;
    }

    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.print("Time set from NTP: ");
    Serial.println(timeStringBuff);
    return true;
}

bool isTimeValid() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return false;
    }

    if(timeinfo.tm_year + 1900 < 2024) {
        Serial.println("Time not valid yet");
        return false;
    }

    return true;
}
