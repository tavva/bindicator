#include "config_manager.h"

Preferences ConfigManager::preferences;
bool ConfigManager::prefsInitialized = false;
const char* ConfigManager::PREF_NAMESPACE = "system";
const char* ConfigManager::KEY_WIFI_SSID = "wifi_ssid";
const char* ConfigManager::KEY_WIFI_PASS = "wifi_pass";
const char* ConfigManager::KEY_FORCED_SETUP = "force_setup";
const char* ConfigManager::KEY_CALENDAR_ID = "calendar_id";

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

bool ConfigManager::isInForcedSetupMode() {
    begin();
    return !preferences.getString(KEY_FORCED_SETUP, "").isEmpty();
}

void ConfigManager::setForcedSetupFlag(const String& flag) {
    begin();
    preferences.putString(KEY_FORCED_SETUP, flag);
}

void ConfigManager::processSetupFlag() {
    begin();
    String flag = preferences.getString(KEY_FORCED_SETUP, "");
    if (flag.isEmpty()) {
        return;
    } else if (flag == "restart-in-setup-mode") {
        setForcedSetupFlag("in-setup-mode");
    } else if (flag == "in-setup-mode") {
        preferences.remove(KEY_FORCED_SETUP);
    }
}

String ConfigManager::getCalendarId() {
    begin();
    return preferences.getString(KEY_CALENDAR_ID, "primary");
}

bool ConfigManager::setCalendarId(const String& id) {
    begin();
    return preferences.putString(KEY_CALENDAR_ID, id);
}
