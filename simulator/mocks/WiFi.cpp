// ABOUTME: WiFi mock for simulator
// ABOUTME: Simulates WiFi connection state without actual network operations

#include "WiFi.h"
#include <cstdlib>

WiFiClass WiFi;

void WiFiClass::begin(const char* ssid, const char* password) {
    const char* mockMode = getenv("SIMULATOR_MOCK");
    if (mockMode && std::string(mockMode) == "1") {
        currentStatus = WL_CONNECTED;
        Serial.println("WiFi connected (mock mode)");
    } else {
        // In real mode, we don't actually connect to WiFi
        // but pretend we're connected for HTTP operations
        currentStatus = WL_CONNECTED;
        Serial.println("WiFi connected (simulator mode)");
    }
}

wl_status_t WiFiClass::status() {
    return currentStatus;
}

IPAddress WiFiClass::localIP() {
    return IPAddress(192, 168, 1, 100);
}

void WiFiClass::reconnect() {
    currentStatus = WL_CONNECTED;
}

bool WiFiClass::softAP(const char* ssid, const char* password) {
    Serial.print("Created AP: ");
    Serial.println(ssid);
    return true;
}
