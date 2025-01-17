#include "button_handler.h"
#include "bindicator.h"
#include "config_manager.h"

ButtonHandler::ButtonHandler(uint8_t pin, unsigned long longPressTime)
    : buttonPin(pin), longPressTime(longPressTime) {}

void ButtonHandler::begin() {
    pinMode(buttonPin, INPUT_PULLUP);
    state.mutex = xSemaphoreCreateMutex();

    xTaskCreate(
        buttonTask,          // Task function
        "ButtonTask",        // Name for debugging
        2048,               // Stack size (bytes)
        this,               // Pass the instance pointer as parameter
        1,                  // Priority
        NULL                // Task handle
    );
}

void ButtonHandler::buttonTask(void *parameter) {
    ButtonHandler* handler = static_cast<ButtonHandler*>(parameter);
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t interval = pdMS_TO_TICKS(20);

    while (true) {
        if (xSemaphoreTake(handler->state.mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            handler->state.currentState = digitalRead(handler->buttonPin);

            // Button press detected
            if (handler->state.currentState == LOW && handler->state.lastState == HIGH) {
                handler->state.pressedTime = millis();
                handler->state.isLongPressHandled = false;
            }
            // Button is being held
            else if (handler->state.currentState == LOW && !handler->state.isLongPressHandled) {
                if (millis() - handler->state.pressedTime >= handler->longPressTime) {
                    handler->onLongPress();
                    handler->state.isLongPressHandled = true;
                }
            }
            // Button release detected
            else if (handler->state.currentState == HIGH && handler->state.lastState == LOW) {
                if (!handler->state.isLongPressHandled &&
                    (millis() - handler->state.pressedTime < handler->longPressTime)) {
                    handler->onShortPress();
                }
            }

            handler->state.lastState = handler->state.currentState;
            xSemaphoreGive(handler->state.mutex);
        }

        vTaskDelayUntil(&lastWakeTime, interval);
    }
}

// Main button

void BindicatorButton::onShortPress() {
    Serial.println("Short press detected");
    Bindicator::handleButtonPress();
}

void BindicatorButton::onLongPress() {
    Serial.println("Long press detected - entering setup mode");
    ConfigManager::setForcedSetupFlag("restart-in-setup-mode");
    ESP.restart();
}
