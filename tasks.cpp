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

    static bool isLoading = true;
    static bool isError = false;
    static int loadingPos = 0;
    static int x = 0;
    static uint8_t r = 0, g = 0, b = 30;
    static uint8_t dot_r = 0, dot_g = 0, dot_b = 0;
    static int animationCounter = 0;
    const int ANIMATION_SPEED = 4;

    const uint8_t exclamation[8][8] = {
        {0,0,0,0,0,0,0,0},
        {0,0,0,1,1,0,0,0},
        {0,0,0,1,1,0,0,0},
        {0,0,0,1,1,0,0,0},
        {0,0,0,1,1,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,1,1,0,0,0},
        {0,0,0,0,0,0,0,0}
    };

    while(true) {
        Command cmd = CMD_NONE;
        if (xQueueReceive(commandQueue, &cmd, 0) == pdTRUE) {
            Serial.printf("Animation received command: %d\n", cmd);
            isLoading = false;
            isError = false;

            switch(cmd) {
                case CMD_SHOW_RECYCLING:
                    Serial.println("Switching to green (recycling)");
                    r = 0; g = 50; b = 0;
                    break;
                case CMD_SHOW_RUBBISH:
                    Serial.println("Switching to brown (rubbish)");
                    r = 20; g = 40; b = 0;
                    break;
                case CMD_SHOW_NEITHER:
                    Serial.println("Switching to blue (neither)");
                    r = 0; g = 0; b = 50;
                    break;
                case CMD_SHOW_LOADING:
                    Serial.println("Showing loading animation");
                    isLoading = true;
                    break;
                case CMD_SHOW_ERROR_API:
                    Serial.println("Showing API error");
                    isError = true;
                    r = 50; g = 0; b = 0;
                    dot_r = 0; dot_g = 0; dot_b = 50;
                    break;
                case CMD_SHOW_ERROR_WIFI:
                    Serial.println("Showing WiFi error");
                    isError = true;
                    r = 50; g = 0; b = 0;
                    dot_r = 50; dot_g = 50; dot_b = 0;  // yellow
                    break;
                case CMD_SHOW_ERROR_OTHER:
                    Serial.println("Showing other error");
                    isError = true;
                    r = 50; g = 0; b = 0;
                    dot_r = 50; dot_g = 0; dot_b = 50;  // purple
                    break;
                default:
                    Serial.printf("Unknown command: %d\n", cmd);
                    break;
            }
        }

        extern DisplayHandler display;
        display.matrix.clear();

        if (isError) {
            uint8_t brightness = (x < 32) ? x * 2 : (64 - x) * 2;
            x = (x + 1) % 64;

            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    if (exclamation[row][col]) {
                        if (row == 6) {  // dot
                            display.matrix.setPixelColor(row * 8 + col,
                                display.matrix.Color(
                                    (dot_r * brightness) / 64,
                                    (dot_g * brightness) / 64,
                                    (dot_b * brightness) / 64
                                )
                            );
                        } else {  // stroke
                            display.matrix.setPixelColor(row * 8 + col,
                                display.matrix.Color(
                                    (r * brightness) / 64,
                                    (g * brightness) / 64,
                                    (b * brightness) / 64
                                )
                            );
                        }
                    }
                }
            }
        } else if (isLoading) {
            int pos1_row, pos1_col, pos2_row, pos2_col;

            if (loadingPos < 6) {
                pos1_row = 1;
                pos1_col = loadingPos + 1;
            } else if (loadingPos < 12) {
                pos1_row = loadingPos - 4;
                pos1_col = 6;
            } else if (loadingPos < 18) {
                pos1_row = 6;
                pos1_col = 17 - loadingPos;
            } else {
                pos1_row = 23 - loadingPos;
                pos1_col = 1;
            }

            int oppositePos = (loadingPos + 12) % 24;
            if (oppositePos < 6) {
                pos2_row = 1;
                pos2_col = oppositePos + 1;
            } else if (oppositePos < 12) {
                pos2_row = oppositePos - 4;
                pos2_col = 6;
            } else if (oppositePos < 18) {
                pos2_row = 6;
                pos2_col = 17 - oppositePos;
            } else {
                pos2_row = 23 - oppositePos;
                pos2_col = 1;
            }

            display.matrix.setPixelColor(pos1_row * 8 + pos1_col, display.matrix.Color(30, 30, 30));
            display.matrix.setPixelColor(pos2_row * 8 + pos2_col, display.matrix.Color(30, 30, 30));

            animationCounter++;
            if (animationCounter >= ANIMATION_SPEED) {
                animationCounter = 0;
                loadingPos = (loadingPos + 1) % 24;
            }
        } else {
            uint8_t brightness = (x < 32) ? x * 2 : (64 - x) * 2;

            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    if(Matrix_Data[row][col] == 1) {
                        display.matrix.setPixelColor(row*8+col,
                            display.matrix.Color(
                                (r * brightness) / 64,
                                (g * brightness) / 64,
                                (b * brightness) / 64
                            )
                        );
                    }
                }
            }
            x = (x + 1) % 64;
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
