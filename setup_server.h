#ifndef SETUP_SERVER_H
#define SETUP_SERVER_H

#include <WebServer.h>
#include <ArduinoJson.h>
#include "oauth_handler.h"
#include "config_manager.h"

class SetupServer {
    public:
        SetupServer(OAuthHandler& oauth);
        ~SetupServer();
        void begin();
        void handleClient();
        bool isConfigured();
        WebServer* server;

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
        const char* getSetupPage();
        void handleRestart();
        void handleFactoryReset();
};

#endif
