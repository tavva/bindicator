#pragma once

#include "Arduino.h"
#include <cstdlib>

class ESPClass {
public:
    void restart() {
        Serial.println("ESP restarting...");
        exit(0);
    }
};

extern ESPClass ESP;
