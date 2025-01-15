#include "tasks.h"
#include "display_handler.h"
#include "wifi_config.h"
#include "secrets.h"
#include "calendar_handler.h"
#include "time_manager.h"
#include "animations.h"
#include <WiFi.h>

uint8_t Matrix_Data[8][8];
uint8_t RGB_Data1[64][3];

QueueHandle_t commandQueue;

void animationTask(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(30);

    Serial.println("Animation task started");

    static bool isLoading = true;
    static bool isError = false;
    static int loadingPos = 0;
    static Color color = Animations::DEFAULT_BLUE;
    static Color dotColor;
    static int animationCounter = 0;
    const int ANIMATION_SPEED = 4;

    extern DisplayHandler display;

    while(true) {
        Command cmd = CMD_NONE;
        if (xQueueReceive(commandQueue, &cmd, 0) == pdTRUE) {
            Serial.printf("Animation received command: %d\n", cmd);
            isLoading = false;
            isError = false;

            switch(cmd) {
                case CMD_SHOW_RECYCLING:
                    Serial.println("Switching to green (recycling)");
                    color = Animations::RECYCLING_GREEN;
                    break;
                case CMD_SHOW_RUBBISH:
                    Serial.println("Switching to brown (rubbish)");
                    color = Animations::RUBBISH_BROWN;
                    break;
                case CMD_SHOW_NEITHER:
                    Serial.println("Switching to blue (neither)");
                    color = Animations::DEFAULT_BLUE;
                    break;
                case CMD_SHOW_LOADING:
                    Serial.println("Showing loading animation");
                    isLoading = true;
                    break;
                case CMD_SHOW_ERROR_API:
                    Serial.println("Showing API error");
                    isError = true;
                    Animations::drawError(display, ErrorType::API);
                    break;
                case CMD_SHOW_ERROR_WIFI:
                    Serial.println("Showing WiFi error");
                    isError = true;
                    Animations::drawError(display, ErrorType::WIFI);
                    break;
                case CMD_SHOW_ERROR_OTHER:
                    Serial.println("Showing other error");
                    isError = true;
                    Animations::drawError(display, ErrorType::OTHER);
                    break;
                default:
                    Serial.printf("Unknown command: %d\n", cmd);
                    break;
            }
        }

        display.matrix.clear();

        if (isError) {
            Animations::drawError(display, ErrorType::API);
        } else if (isLoading) {
            Animations::drawLoading(display);
        } else {
            Animations::drawPulse(display, color);
        }

        display.matrix.show();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void wifiTask(void* parameter) {
    while(true) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected, attempting to reconnect...");
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

            Command cmd = CMD_WIFI_DISCONNECTED;
            xQueueSend(commandQueue, &cmd, 0);

            while (WiFi.status() != WL_CONNECTED) {
                vTaskDelay(pdMS_TO_TICKS(500));
            }

            Serial.println("WiFi connected!");
            cmd = CMD_WIFI_CONNECTED;
            xQueueSend(commandQueue, &cmd, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void calendarTask(void* parameter) {
    extern CalendarHandler calendar;

    Serial.println("Calendar task started");

    Command cmd = CMD_SHOW_LOADING;
    xQueueSend(commandQueue, &cmd, 0);

    // Delay to let wifi connect
    vTaskDelay(pdMS_TO_TICKS(5000));

    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Waiting for time sync...");
        setupTime();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    Serial.println("Time synchronized!");

    while(true) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Checking calendar...");
            bool hasRecycling = false;
            bool hasRubbish = false;

            if (calendar.checkForBinEvents(hasRecycling, hasRubbish)) {
                Command cmd;

                if (hasRecycling && hasRubbish) {
                    Serial.println("Both bins!");
                    cmd = CMD_SHOW_RECYCLING;
                } else if (hasRecycling) {
                    Serial.println("Recycling only");
                    cmd = CMD_SHOW_RECYCLING;
                } else if (hasRubbish) {
                    Serial.println("Rubbish only");
                    cmd = CMD_SHOW_RUBBISH;
                } else {
                    Serial.println("No bins");
                    cmd = CMD_SHOW_NEITHER;
                }

                if (xQueueSend(commandQueue, &cmd, 0) != pdTRUE) {
                    Serial.println("Failed to send command to queue!");
                }
            } else {
                Serial.println("Failed to check calendar");
            }
        } else {
            Serial.println("WiFi not connected, skipping calendar check");
        }

        vTaskDelay(pdMS_TO_TICKS(3600000));
    }
}
