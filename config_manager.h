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

        static bool isInForcedSetupMode();
        static void setForcedSetupFlag(const String& flag);
        static void processSetupFlag();

    private:
        static const char* PREF_NAMESPACE;
        static const char* KEY_WIFI_SSID;
        static const char* KEY_WIFI_PASS;
        static const char* KEY_FORCED_SETUP;

        static Preferences preferences;
        static bool prefsInitialized;
};

#endif
