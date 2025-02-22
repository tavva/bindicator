#include "time_manager.h"
#include <time.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;      // GMT+0
const int   daylightOffset_sec = 3600; // +1 hour for BST

bool setupTime() {
    Serial.println("Setting up time sync...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    const int maxRetries = 5;
    const int baseDelay = 1000; // Start with 1 second delay

    for (int retry = 0; retry < maxRetries; retry++) {
        struct tm timeinfo;
        if(getLocalTime(&timeinfo)) {
            char timeStringBuff[50];
            strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
            Serial.print("Time set from NTP: ");
            Serial.println(timeStringBuff);
            return true;
        }

        Serial.printf("Failed to obtain time from NTP (attempt %d of %d)\n", retry + 1, maxRetries);

        if (retry < maxRetries - 1) {
            int delayTime = baseDelay * (1 << retry); // Exponential backoff
            Serial.printf("Retrying in %d ms...\n", delayTime);
            vTaskDelay(pdMS_TO_TICKS(delayTime));
        }
    }

    Serial.println("Failed to obtain time after all retries");
    return false;
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
