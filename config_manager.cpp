#include "config_manager.h"

Preferences ConfigManager::preferences;
const char* ConfigManager::PREF_NAMESPACE = "system";
const char* ConfigManager::KEY_WIFI_SSID = "wifi_ssid";
const char* ConfigManager::KEY_WIFI_PASS = "wifi_pass";
const char* ConfigManager::KEY_FORCE_SETUP = "force_setup";
const char* ConfigManager::KEY_CALENDAR_ID = "calendar_id";
const char* ConfigManager::KEY_BIN_TAKEN_OUT = "bin_taken";
const char* ConfigManager::KEY_BIN_TYPE = "bin_type";
const char* ConfigManager::KEY_STATE = "state";
const char* ConfigManager::KEY_COMPLETED_TIME = "completed_time";

void ConfigManager::begin() {
    preferences.begin(PREF_NAMESPACE, false);
}

bool ConfigManager::isConfigured() {
    begin();
    String ssid = preferences.getString(KEY_WIFI_SSID, "");
    String password = preferences.getString(KEY_WIFI_PASS, "");
    return !ssid.isEmpty() && !password.isEmpty();
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
    return preferences.getString(KEY_FORCE_SETUP, "") == "restart-in-setup-mode";
}

void ConfigManager::setForcedSetupFlag(const String& flag) {
    begin();
    preferences.putString(KEY_FORCE_SETUP, flag);
}

void ConfigManager::processSetupFlag() {
    begin();
    String flag = preferences.getString(KEY_FORCE_SETUP, "");
    if (!flag.isEmpty()) {
        preferences.putString(KEY_FORCE_SETUP, "");
        preferences.remove(KEY_FORCE_SETUP);
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

time_t ConfigManager::getBinTakenOutTime() {
    begin();
    return preferences.getInt(KEY_BIN_TAKEN_OUT, 0);
}

bool ConfigManager::setBinTakenOutTime(time_t time) {
    begin();
    return preferences.putInt(KEY_BIN_TAKEN_OUT, time);
}

BinType ConfigManager::getBinType() {
    begin();
    return static_cast<BinType>(preferences.getInt(KEY_BIN_TYPE, static_cast<int>(BinType::NONE)));
}

bool ConfigManager::setBinType(BinType type) {
    begin();
    return preferences.putInt(KEY_BIN_TYPE, static_cast<int>(type));
}

int ConfigManager::getState() {
    begin();
    return preferences.getInt(KEY_STATE, 0);  // 0 = NO_COLLECTION
}

bool ConfigManager::setState(int state) {
    begin();
    return preferences.putInt(KEY_STATE, state);
}

time_t ConfigManager::getCompletedTime() {
    begin();
    return preferences.getInt(KEY_COMPLETED_TIME, 0);
}

bool ConfigManager::setCompletedTime(time_t time) {
    begin();
    return preferences.putInt(KEY_COMPLETED_TIME, time);
}

#ifdef TESTING
void ConfigManager::clearForTesting() {
    begin();
    preferences.remove(KEY_WIFI_SSID);
    preferences.remove(KEY_WIFI_PASS);
    preferences.remove(KEY_FORCE_SETUP);
    preferences.remove(KEY_CALENDAR_ID);
    preferences.remove(KEY_BIN_TAKEN_OUT);
    preferences.remove(KEY_BIN_TYPE);
    preferences.remove(KEY_STATE);
    preferences.remove(KEY_COMPLETED_TIME);
}
#endif
