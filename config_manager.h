#pragma once

#include <Arduino.h>
#ifdef ESP32
    #include <Preferences.h>
#else
    #include "Preferences.h"
#endif
#include "bin_type.h"

class ConfigManager {
    public:
        static void begin();
        static void processSetupFlag();
        static bool isConfigured();
        static bool isInForcedSetupMode();
        static void setForcedSetupFlag(const String& flag);

        static String getCalendarId();
        static bool setCalendarId(const String& id);

        static int getState();
        static bool setState(int state);
        static time_t getCompletedTime();
        static bool setCompletedTime(time_t time);

        static String getWifiSSID();
        static String getWifiPassword();
        static bool setWifiCredentials(const String& ssid, const String& password);

        static time_t getBinTakenOutTime();
        static bool setBinTakenOutTime(time_t time);

        static BinType getBinType();
        static bool setBinType(BinType type);

        #ifdef TESTING
        static void clearForTesting();
        #endif

    private:
        static Preferences preferences;
        static const char* PREF_NAMESPACE;
        static const char* KEY_CALENDAR_ID;
        static const char* KEY_STATE;
        static const char* KEY_COMPLETED_TIME;
        static const char* KEY_FORCE_SETUP;
        static const char* KEY_WIFI_SSID;
        static const char* KEY_WIFI_PASS;
        static const char* KEY_BIN_TAKEN_OUT;
        static const char* KEY_BIN_TYPE;
};
