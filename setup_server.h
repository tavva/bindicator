#ifndef SETUP_SERVER_H
#define SETUP_SERVER_H

#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "oauth_handler.h"

class SetupServer {
    public:
        SetupServer(OAuthHandler& oauth);
        ~SetupServer();
        void begin();
        void handleClient();
        bool isConfigured();
        WebServer* server;

        String getWifiSSID() { return config.wifi_ssid; }
        String getWifiPassword() { return config.wifi_password; }

    private:
        OAuthHandler& oauthHandler;

        struct Config {
            String wifi_ssid;
            String wifi_password;
        };
        Config config;

        void handleRoot();
        void handleSave();
        void handleOAuth();

        bool loadConfig();
        bool saveConfig();

        const char* getSetupPage();

        static const char* CONFIG_FILE;
};

#endif
