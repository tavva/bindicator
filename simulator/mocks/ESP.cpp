// ABOUTME: ESP32 system functions mock for simulator
// ABOUTME: Provides ESP restart capability by exiting simulator process

#include "ESP.h"
#include "Arduino.h"

ESPClass ESP;

void ESPClass::restart() {
    Serial.println("ESP restarting...");
    exit(0);
}
