#pragma once

#include "Arduino.h"

typedef enum {
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL,
    WL_CONNECTED,
    WL_CONNECT_FAILED,
    WL_CONNECTION_LOST,
    WL_DISCONNECTED
} wl_status_t;

class IPAddress {
public:
    IPAddress() : addr{192, 168, 1, 100} {}
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : addr{a, b, c, d} {}

    String toString() const {
        return std::to_string(addr[0]) + "." + std::to_string(addr[1]) + "." +
               std::to_string(addr[2]) + "." + std::to_string(addr[3]);
    }

private:
    uint8_t addr[4];
};

class WiFiClass {
public:
    void mode(uint8_t mode) {}
    void setHostname(const char* hostname) {}
    void begin(const char* ssid, const char* password);
    wl_status_t status();
    IPAddress localIP();
    void reconnect();
    bool softAP(const char* ssid, const char* password);
    bool softAP(const char* ssid, const String& password) {
        return softAP(ssid, password.c_str());
    }

private:
    wl_status_t currentStatus = WL_DISCONNECTED;
};

extern WiFiClass WiFi;

#define WIFI_AP_STA 3
#define WIFI_AP 2
#define WIFI_STA 1
