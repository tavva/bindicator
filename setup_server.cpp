#include "setup_server.h"
#include <WiFi.h>
#include "config_manager.h"
#include "calendar_handler.h"

SetupServer::SetupServer(OAuthHandler& oauth)
    : oauthHandler(oauth), server(nullptr) {
}

SetupServer::~SetupServer() {
    if (server != nullptr) {
        delete server;
    }
}

void SetupServer::begin() {
    if (server == nullptr) {
        server = new WebServer(80);
    }

    config.wifi_ssid = ConfigManager::getWifiSSID();
    config.wifi_password = ConfigManager::getWifiPassword();

    server->on("/", HTTP_GET, std::bind(&SetupServer::handleRoot, this));
    server->on("/save", HTTP_POST, std::bind(&SetupServer::handleSave, this));
    server->on("/oauth", HTTP_GET, [this]() {
        String authUrl = oauthHandler.getAuthUrl();
        server->sendHeader("Location", authUrl);
        server->send(302);
    });
    server->on("/oauth_callback", HTTP_GET, std::bind(&SetupServer::handleOAuth, this));
    server->on("/restart", HTTP_POST, [this]() { handleRestart(); });
    server->on("/factory-reset", HTTP_POST, [this]() { handleFactoryReset(); });
    server->on("/save-calendar", HTTP_POST, std::bind(&SetupServer::handleSaveCalendar, this));
    server->on("/calendars", HTTP_GET, [this]() { this->handleCalendarList(); });
    server->on("/upcoming-bins", HTTP_GET, [this]() { this->handleUpcomingBins(); });

    server->begin();
    Serial.println("Setup server started");
}

void SetupServer::handleClient() {
    server->handleClient();
}

void SetupServer::handleSave() {
    if (server->hasArg("ssid") && server->hasArg("password")) {
        config.wifi_ssid = server->arg("ssid");
        config.wifi_password = server->arg("password");

        if (ConfigManager::setWifiCredentials(config.wifi_ssid, config.wifi_password)) {
            ConfigManager::setForcedSetupFlag("restart-in-setup-mode");

            String html = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi Settings Saved</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 0 auto;
            padding: 20px;
            text-align: center;
        }
        @media (max-width: 640px) {
            body { padding: 15px; }
            h1 { font-size: 24px; }
        }
        .message {
            margin: 20px 0;
            padding: 20px;
            border: 1px solid #4CAF50;
            border-radius: 5px;
        }
        .urls { margin: 20px 0; }
        .url {
            font-family: monospace;
            background: #f5f5f5;
            padding: 5px 10px;
            border-radius: 3px;
            word-break: break-all;
        }
        @media (max-width: 400px) {
            .message { padding: 15px; }
            h1 { font-size: 22px; }
        }
    </style>
</head>
<body>
    <h1>WiFi Settings Saved</h1>
    <div class="message">
        <p>WiFi configuration has been saved.</p>
        <p>The device will now restart and enter setup mode to complete configuration.</p>
        <p>When it's ready, you can complete setup at:</p>
        <p class="url"><a href="http://bindicator.local">http://bindicator.local</a></p>
        <p><small>(If bindicator.local doesn't work, you may need to find the device's IP address. The default is 192.168.4.1, but it may be different depending on your router.)</small></p>
    </div>
    <p>The device will restart in 5 seconds...</p>
    <script>
        setTimeout(function() {
            document.body.innerHTML += '<p>Restarting...</p>';
        }, 4000);
    </script>
</body>
</html>)html";

            server->send(200, "text/html", html);
            delay(5000);
            ESP.restart();
        } else {
            server->send(500, "text/plain", "Failed to save configuration");
        }
    } else {
        server->send(400, "text/plain", "Missing parameters");
    }
}

void SetupServer::handleRoot() {
    server->send(200, "text/html", getSetupPage());
}

void SetupServer::handleOAuth() {
    if (server->hasArg("code")) {
        String code = server->arg("code");
        String error;

        if (oauthHandler.exchangeAuthCode(code, error)) {
            server->sendHeader("Location", "/");
            server->send(302);
        } else {
            server->send(400, "text/plain", "Failed to exchange auth code: " + error);
        }
    } else {
        server->send(400, "text/plain", "No auth code provided");
    }
}

void SetupServer::handleRestart() {
    server->send(200, "text/plain", "Restarting...");
    delay(1000);
    ESP.restart();
}

void SetupServer::handleFactoryReset() {
    server->send(200, "text/plain", "Performing factory reset...");

    Preferences prefs;
    prefs.begin("system", false);
    prefs.clear();
    prefs.end();

    prefs.begin("oauth", false);
    prefs.clear();
    prefs.end();

    delay(1000);
    ESP.restart();
}

void SetupServer::handleSaveCalendar() {
    if (server->hasArg("calendar_id")) {
        String calendarId = server->arg("calendar_id");

        if (ConfigManager::setCalendarId(calendarId)) {
            String html = R"html(
<!DOCTYPE html>
<html>
<head>
    <title>Calendar Settings Saved</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; text-align: center; }
        .message { margin: 20px 0; padding: 20px; border: 1px solid #4CAF50; border-radius: 5px; }
    </style>
</head>
<body>
    <h1>Calendar Settings Saved</h1>
    <div class="message">
        <p>Calendar ID has been updated successfully.</p>
    </div>
    <p><a href="/">Return to Setup</a></p>
</body>
</html>)html";

            server->send(200, "text/html", html);
        } else {
            server->send(500, "text/plain", "Failed to save calendar ID");
        }
    } else {
        server->send(400, "text/plain", "Missing calendar ID");
    }
}

void SetupServer::handleCalendarList() {
    if (!oauthHandler.isAuthorized()) {
        server->send(401, "application/json", "{\"error\":\"Not authorized\"}");
        return;
    }

    StaticJsonDocument<4096> calendars;
    CalendarHandler calendarHandler(oauthHandler);

    if (calendarHandler.getAvailableCalendars(calendars)) {
        String response;
        serializeJson(calendars, response);
        server->send(200, "application/json", response);
    } else {
        server->send(500, "application/json", "{\"error\":\"Failed to fetch calendars\"}");
    }
}

void SetupServer::handleUpcomingBins() {
    if (!oauthHandler.isAuthorized()) {
        server->send(401, "application/json", "{\"error\":\"Not authorized\"}");
        return;
    }

    StaticJsonDocument<4096> events;
    CalendarHandler calendarHandler(oauthHandler);

    if (calendarHandler.getUpcomingBinDays(events)) {
        String response;
        serializeJson(events, response);
        server->send(200, "application/json", response);
    } else {
        server->send(500, "application/json", "{\"error\":\"Failed to fetch events\"}");
    }
}
