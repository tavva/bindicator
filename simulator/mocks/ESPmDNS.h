#pragma once

#include "Arduino.h"

class MDNSClass {
public:
    bool begin(const char* hostname) {
        Serial.print("mDNS started: ");
        Serial.print(hostname);
        Serial.println(".local");
        return true;
    }

    void addService(const char* service, const char* proto, uint16_t port) {
        Serial.print("mDNS service added: ");
        Serial.print(service);
        Serial.print(".");
        Serial.println(proto);
    }
};

extern MDNSClass MDNS;
