#include "tasks.h"
#include "display_handler.h"
#include "secrets.h"
#include "calendar_handler.h"
#include "time_manager.h"
#include "animations.h"
#include "bin_type.h"
#include "bindicator.h"
#include <WiFi.h>

uint8_t Matrix_Data[8][8];
uint8_t RGB_Data1[64][3];

QueueHandle_t commandQueue;

const uint32_t CALENDAR_CHECK_INTERVAL_MS = 3600000;  // 1 hour

extern CalendarHandler calendar;

void animationTask(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(30);

    Serial.println("Animation task started");

    static bool isLoading = true;
    static bool isSetupMode = false;
    static bool isError = false;
    static bool isBin = false;
    static bool isComplete = false;
    static Color color = Animations::DEFAULT_BLUE;

    extern DisplayHandler display;

    while(true) {
        Command cmd = CMD_NONE;
        if (xQueueReceive(commandQueue, &cmd, 0) == pdTRUE) {
            Serial.printf("Animation received command: %d\n", cmd);
            isLoading = false;
            isError = false;
            isBin = false;
            isComplete = false;

            switch(cmd) {
                case CMD_SHOW_RECYCLING:
                    Serial.println("Switching to green (recycling)");
                    isBin = true;
                    color = Animations::RECYCLING_GREEN;
                    break;
                case CMD_SHOW_RUBBISH:
                    Serial.println("Switching to brown (rubbish)");
                    isBin = true;
                    color = Animations::RUBBISH_BROWN;
                    break;
                case CMD_SHOW_NEITHER:
                    Serial.println("Switching to blue (neither)");
                    color = Animations::DEFAULT_BLUE;
                    break;
                case CMD_SHOW_COMPLETED:
                    Serial.println("Showing completed animation");
                    isComplete = true;
                    color = Animations::COMPLETE_GREEN;
                    break;
                case CMD_SHOW_LOADING:
                    Serial.println("Showing loading animation");
                    isLoading = true;
                    break;
                case CMD_SHOW_SETUP_MODE:
                    Serial.println("Showing setup mode");
                    isSetupMode = true;
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
        } else if (isSetupMode) {
            Animations::drawSetupMode(display);
        } else if (isBin) {
            Animations::drawBinImage(display, color);
        } else if (isComplete) {
            Animations::drawComplete(display, color);
        } else {
            Animations::drawPulse(display, color);
        }

        display.matrix.show();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void calendarTask(void* parameter) {
    const TickType_t xDelay = pdMS_TO_TICKS(CALENDAR_CHECK_INTERVAL_MS);

    Command cmd = CMD_SHOW_LOADING;
    xQueueSend(commandQueue, &cmd, 0);

    // Initial delay to allow system to stabilize and load state
    vTaskDelay(pdMS_TO_TICKS(5000));

    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Waiting for time sync...");
        setupTime();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    Serial.println("Time synced");

    while (true) {
        while (WiFi.status() != WL_CONNECTED) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        Serial.println("\nCalendar check cycle starting");

        if (!Bindicator::shouldCheckCalendar()) {
            Serial.println("Skipping calendar check - bin already taken out or wrong time");
            vTaskDelay(xDelay);
            continue;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Checking calendar...");
            bool hasRecycling = false;
            bool hasRubbish = false;

            if (calendar.checkForBinEvents(hasRecycling, hasRubbish)) {
                CollectionState state = CollectionState::NO_COLLECTION;
                if (hasRecycling) {
                    state = CollectionState::RECYCLING_DUE;
                } else if (hasRubbish) {
                    state = CollectionState::RUBBISH_DUE;
                }
                Bindicator::updateFromCalendar(state);
            } else {
                Serial.println("Failed to check calendar");
                Bindicator::setErrorState(ErrorType::API);
            }
        } else {
            Serial.println("WiFi not connected, skipping calendar check");
            Bindicator::setErrorState(ErrorType::WIFI);
        }

        vTaskDelay(xDelay);
    }
}
