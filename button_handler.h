#pragma once

#include <Arduino.h>

class ButtonHandler {
public:
    ButtonHandler(uint8_t pin, unsigned long longPressTime = 3000);
    void begin();

    virtual void onShortPress() {};
    virtual void onLongPress() {};

private:
    static void buttonTask(void *parameter);

    const uint8_t buttonPin;
    const unsigned long longPressTime;

    struct ButtonState {
        bool lastState = HIGH;
        bool currentState = HIGH;
        unsigned long pressedTime = 0;
        bool isLongPressHandled = false;
        SemaphoreHandle_t mutex;
    };

    ButtonState state;
};

// Main button

class BindicatorButton : public ButtonHandler {
public:
    BindicatorButton(uint8_t pin, unsigned long longPressTime = 3000)
        : ButtonHandler(pin, longPressTime) {}

    void onShortPress() override;
    void onLongPress() override;
};
