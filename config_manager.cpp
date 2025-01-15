#include "config_manager.h"

Preferences ConfigManager::preferences;
bool ConfigManager::prefsInitialized = false;
const char* ConfigManager::PREF_NAMESPACE = "system";
const char* ConfigManager::KEY_WIFI_SSID = "wifi_ssid";
const char* ConfigManager::KEY_WIFI_PASS = "wifi_pass";

void ConfigManager::begin() {
    if (!prefsInitialized) {
        preferences.begin(PREF_NAMESPACE, false);
        prefsInitialized = true;
    }
}

bool ConfigManager::isConfigured() {
    begin();
    return !getWifiSSID().isEmpty();
}

String ConfigManager::getWifiSSID() {
    begin();
    return preferences.getString(KEY_WIFI_SSID, "");
}

String ConfigManager::getWifiPassword() {
    begin();
    return preferences.getString(KEY_WIFI_PASS, "");
}

bool ConfigManager::setWifiCredentials(const String& ssid, const String& password) {
    begin();
    bool success = preferences.putString(KEY_WIFI_SSID, ssid) &&
                  preferences.putString(KEY_WIFI_PASS, password);

    if (success) {
        Serial.println("WiFi credentials saved successfully");
    } else {
        Serial.println("Failed to save WiFi credentials");
    }
    return success;
}
