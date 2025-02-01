#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include "bin_type.h"
#ifdef ESP32
    #include <Preferences.h>
#else
    #include "Preferences.h"
#endif

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

        static String getCalendarId();
        static bool setCalendarId(const String& id);

        static time_t getBinTakenOutTime();
        static bool setBinTakenOutTime(time_t time);

        static BinType getBinType();
        static bool setBinType(BinType type);

    private:
        static const char* PREF_NAMESPACE;
        static const char* KEY_WIFI_SSID;
        static const char* KEY_WIFI_PASS;
        static const char* KEY_FORCED_SETUP;
        static const char* KEY_CALENDAR_ID;
        static const char* KEY_BIN_TAKEN_OUT;
        static const char* KEY_BIN_TYPE;

        static Preferences preferences;
        static bool prefsInitialized;
};

#endif
