#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <WiFi.h>
#include "wifi_config.h"
#include "time_manager.h"
#include "oauth_handler.h"
#include "calendar_handler.h"
#include "secrets.h"
#include "display_handler.h"
#include "tasks.h"

OAuthHandler oauth(CLIENT_ID, CLIENT_SECRET, REFRESH_TOKEN);
CalendarHandler calendar(oauth, CALENDAR_ID);
DisplayHandler display;
bool hasRecycling, hasRubbish;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting up...");

    display.begin();
    display.matrix.clear();
    display.matrix.setBrightness(50);
    memset(Matrix_Data, 1, sizeof(Matrix_Data));

    commandQueue = xQueueCreate(10, sizeof(Command));

    xTaskCreate(
        animationTask,
        "Animation",
        10000,
        NULL,
        3,
        NULL
    );

    xTaskCreate(
        wifiTask,
        "WiFi",
        10000,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        calendarTask,
        "Calendar",
        10000,
        NULL,
        1,
        NULL
    );

    Serial.println("Tasks created");
}

void loop() {
    vTaskDelete(NULL);
}
