#include "tasks.h"
#include "display_handler.h"
#include "wifi_config.h"
#include "secrets.h"
#include "calendar_handler.h"
#include "time_manager.h"
#include <WiFi.h>

uint8_t Matrix_Data[8][8];
uint8_t RGB_Data1[64][3];

QueueHandle_t commandQueue;

void animationTask(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(30);

    Serial.println("Animation task started");

    while(true) {
        static int x = 0;
        static uint8_t r = 0, g = 0, b = 30;  // Start with blue (neither)

        Command cmd = CMD_NONE;
        if (xQueueReceive(commandQueue, &cmd, 0) == pdTRUE) {
            Serial.printf("Animation received command: %d\n", cmd);
            switch(cmd) {
                case CMD_SHOW_RECYCLING:
                    Serial.println("Switching to green (recycling)");
                    r = 0; g = 50; b = 0;  // Green
                    break;
                case CMD_SHOW_RUBBISH:
                    Serial.println("Switching to brown (rubbish)");
                    r = 20; g = 40; b = 0;  // Brown
                    break;
                case CMD_SHOW_NEITHER:
                    Serial.println("Switching to blue (neither)");
                    r = 0; g = 0; b = 50;   // Blue
                    break;
                default:
                    Serial.printf("Unknown command: %d\n", cmd);
                    break;
            }
        }

        uint8_t brightness = (x < 32) ? x * 2 : (64 - x) * 2;

        if (x % 32 == 0) {
            Serial.printf("Frame: %d, Brightness: %d, Color(R,G,B): %d,%d,%d\n",
                x, brightness, r, g, b);
        }

        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                RGB_Data1[row*8+col][0] = (r * brightness) / 64;
                RGB_Data1[row*8+col][1] = (g * brightness) / 64;
                RGB_Data1[row*8+col][2] = (b * brightness) / 64;
            }
        }

        extern DisplayHandler display;
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                if(Matrix_Data[row][col] == 1) {
                    uint32_t color = display.matrix.Color(
                        RGB_Data1[row*8+col][0],
                        RGB_Data1[row*8+col][1],
                        RGB_Data1[row*8+col][2]
                    );
                    display.matrix.setPixelColor(row*8+col, color);

                    // Debug first pixel's color every 32 frames
                    if (row == 0 && col == 0 && x % 32 == 0) {
                        Serial.printf("Pixel(0,0) color values: R=%d, G=%d, B=%d\n",
                            RGB_Data1[0][0],
                            RGB_Data1[0][1],
                            RGB_Data1[0][2]
                        );
                    }
                }
            }
        }

        display.matrix.show();
        x = (x + 1) % 64;

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
                    cmd = CMD_SHOW_RECYCLING;  // Priority to recycling
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

        vTaskDelay(pdMS_TO_TICKS(300000));
    }
}
