#include "setup_server.h"
#include <WiFi.h>

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
    <title>WiFi Settings Saved</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; text-align: center; }
        .message { margin: 20px 0; padding: 20px; border: 1px solid #4CAF50; border-radius: 5px; }
        .urls { margin: 20px 0; }
        .url { font-family: monospace; background: #f5f5f5; padding: 5px 10px; border-radius: 3px; }
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

const char* SetupServer::getSetupPage() {
    static String page;

    String wifiStatus = config.wifi_ssid.isEmpty() ?
        "<span class='status-incomplete'>Not Configured</span>" :
        "<span class='status-complete'>Configured (" + config.wifi_ssid + ")" +
        (WiFi.status() == WL_CONNECTED ? " - Connected" : " - Not Connected") + "</span>";

    String oauthStatus = oauthHandler.isAuthorized() ?
        "<span class='status-complete'>Connected</span>" :
        "<span class='status-incomplete'>Not Connected</span>";

    String calendarStatus = "<span class='status-complete'>Configured (" +
        (ConfigManager::getCalendarId() == "primary" ? "Main Calendar" : ConfigManager::getCalendarId()) +
        ")</span>";

    page = F(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
            "<title>Bindicator Setup</title>"
            "<style>"
                "body { font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; background-color: #1e1e1e; color: #ffffff; }"
                ".setup-section { margin-bottom: 30px; padding: 20px; border: 1px solid #333; border-radius: 5px; position: relative; background-color: #252525; }"
                ".setup-section::before { content: ''; position: absolute; top: 0; left: 0; width: 4px; height: 100%; border-radius: 5px 0 0 5px; }"
                ".setup-section.incomplete::before { background-color: #f44336; }"
                ".setup-section.complete::before { background-color: #4CAF50; }"
                ".setup-section.disabled { opacity: 0.7; background-color: #1a1a1a; }"
                "h1 { color: #ffffff; }"
                "h2 { color: #ffffff; margin-top: 0; display: flex; align-items: center; gap: 10px; }"
                ".step-number { background-color: #444; color: white; width: 24px; height: 24px; border-radius: 12px; display: flex; align-items: center; justify-content: center; font-size: 14px; }"
                "label { display: block; margin-bottom: 5px; }"
                "input[type=\"text\"], input[type=\"password\"] { width: 100%; padding: 8px; margin-bottom: 10px; border: 1px solid #444; border-radius: 4px; background-color: #333; color: #ffffff; }"
                "button { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; }"
                "button:hover { background-color: #45a049; }"
                "button:disabled { background-color: #666; cursor: not-allowed; }"
                ".status { margin-top: 10px; }"
                ".status-complete { color: #4CAF50; }"
                ".status-incomplete { color: #f44336; }"
                ".help-text { font-size: 0.9em; color: #888; margin-top: 5px; }"
                ".setup-complete { margin: 40px 0; padding: 30px; border: 2px solid #4CAF50; border-radius: 10px; background-color: #252525; text-align: center; }"
                ".setup-complete h2 { display: block }"
                ".launch-button { background-color: #4CAF50; color: white; padding: 15px 30px; border: none; border-radius: 5px; font-size: 18px; cursor: pointer; margin-top: 20px; }"
                ".launch-button:hover { background-color: #45a049; }"
            "</style>"
        "</head>"
        "<body>"
            "<h1>Bindicator Setup</h1>");

    // WiFi Section
    page += "<div class='setup-section" + String(config.wifi_ssid.isEmpty() ? " incomplete" : " complete") + "'>"
            "<h2><span class='step-number'>1</span> WiFi Setup</h2>"
            "<form action='/save' method='POST'>"
                "<label for='ssid'>WiFi Name:</label>"
                "<input type='text' id='ssid' name='ssid' value='" + config.wifi_ssid + "'>"
                "<label for='password'>WiFi Password:</label>"
                "<input type='password' id='password' name='password' value='" + config.wifi_password + "'>"
                "<button type='submit'>Save WiFi Settings</button>"
            "</form>"
            "<p class='status'>Status: " + wifiStatus + "</p>"
            "</div>";

    // OAuth Section
    String oauthContent = config.wifi_ssid.isEmpty() ?
        "<p>Please complete WiFi setup first</p>" :
        "<p>Click below to connect your Google Calendar:</p><a href='/oauth'><button>Connect Google Calendar</button></a>";

    page += "<div class='setup-section" +
            String(oauthHandler.isAuthorized() ? " complete" : " incomplete") +
            String(config.wifi_ssid.isEmpty() ? " disabled" : "") + "'>" +
            "<h2><span class='step-number'>2</span> Google Calendar Setup</h2>" +
            oauthContent +
            "<p class='status'>Status: " + oauthStatus + "</p>"
            "</div>";

    // Setup complete
    if (WiFi.status() == WL_CONNECTED && oauthHandler.isAuthorized()) {
        page += "<div id='setupComplete' class='setup-complete'>"
                "<h2>Your Bindicator is Ready!</h2>"
                "<p>Setup is complete and your Bindicator is configured.</p>"
                "<button class='launch-button' onclick='restartDevice()'>Launch Bindicator</button>"
                "</div>";
    }

    // Calendar Section
    page += "<div class='setup-section" +
            String(!oauthHandler.isAuthorized() ? " disabled" : " complete") + "'>" +
            "<h2><span class='step-number'>3</span> Calendar Settings</h2>"
            "<form action='/save-calendar' method='POST'>"
                "<label for='calendar_id'>Calendar ID:</label>"
                "<input type='text' id='calendar_id' name='calendar_id' value='" + ConfigManager::getCalendarId() + "'" +
                String(!oauthHandler.isAuthorized() ? " disabled" : "") + ">"
                "<p class='help-text'>Leave as \"primary\" to use your main calendar, or enter a specific calendar ID</p>"
                "<button type='submit'" + String(!oauthHandler.isAuthorized() ? " disabled" : "") + ">Save Calendar Settings</button>"
            "</form>"
            "<p class='status'>Status: " + calendarStatus + "</p>"
            "</div>";

    // Device Control Section
    page += F("<div class='setup-section'>"
            "<h2 style='color: #ff4444;'>Device Control</h2>"
            "<div style='display: flex; gap: 10px;'>"
                "<button onclick='restartDevice()' style='background-color: #ff4444;'>Restart Device</button>"
                "<button onclick='factoryReset()' style='background-color: #ff4444;'>Factory Reset</button>"
            "</div>"
            "</div>"
            "<script>"
                "function restartDevice() {"
                    "if (confirm('Are you sure you want to restart the device?')) {"
                        "fetch('/restart', { method: 'POST' })"
                            ".then(() => alert('Device is restarting...'))"
                            ".catch(err => alert('Error: ' + err));"
                    "}"
                "}"
                "function factoryReset() {"
                    "if (confirm('WARNING: This will erase all settings and return the device to factory defaults.\\n\\nAre you sure you want to continue?')) {"
                        "fetch('/factory-reset', { method: 'POST' })"
                            ".then(() => alert('Device is resetting and will restart...'))"
                            ".catch(err => alert('Error: ' + err));"
                    "}"
                "}"
            "</script>"
        "</body>"
        "</html>");

    return page.c_str();
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

bool SetupServer::isConfigured() {
    return ConfigManager::isConfigured();
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
