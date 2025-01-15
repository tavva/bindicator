#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

class ConfigManager {
    public:
        static void begin();
        static bool isConfigured();

        static String getWifiSSID();
        static String getWifiPassword();
        static bool setWifiCredentials(const String& ssid, const String& password);

    private:
        static const char* PREF_NAMESPACE;  // Will be "system"
        static const char* KEY_WIFI_SSID;   // Will be "wifi_ssid"
        static const char* KEY_WIFI_PASS;   // Will be "wifi_pass"

        static Preferences preferences;
        static bool prefsInitialized;
};

#endif
