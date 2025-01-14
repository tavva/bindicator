#ifndef SETUP_SERVER_H
#define SETUP_SERVER_H

#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

class SetupServer {
    public:
        SetupServer();
        void begin();
        void handleClient();
        bool isConfigured();

        String getWifiSSID() { return config.wifi_ssid; }
        String getWifiPassword() { return config.wifi_password; }

    private:
        WebServer server;

        struct Config {
            String wifi_ssid;
            String wifi_password;
        };
        Config config;

        void handleRoot();
        void handleSave();

        bool loadConfig();
        bool saveConfig();

        const char* getSetupPage();

        static const char* CONFIG_FILE;
};

#endif
