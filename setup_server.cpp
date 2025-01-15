#include "setup_server.h"

const char* SetupServer::CONFIG_FILE = "/config.json";

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

    loadConfig();

    server->on("/", HTTP_GET, std::bind(&SetupServer::handleRoot, this));
    server->on("/save", HTTP_POST, std::bind(&SetupServer::handleSave, this));
    server->on("/oauth", HTTP_GET, [this]() {
        String authUrl = oauthHandler.getAuthUrl();
        server->sendHeader("Location", authUrl);
        server->send(302);
    });
    server->on("/oauth_callback", HTTP_GET, std::bind(&SetupServer::handleOAuth, this));

    server->begin();
    Serial.println("Setup server started");
}

void SetupServer::handleClient() {
    server->handleClient();
}

bool SetupServer::isConfigured() {
    return loadConfig() && !config.wifi_ssid.isEmpty();
}

bool SetupServer::loadConfig() {
    File file = SPIFFS.open(CONFIG_FILE, "r");
    if(!file) {
        Serial.println("No configuration file found");
        return false;
    }

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if(error) {
        Serial.println("Failed to parse config file");
        return false;
    }

    config.wifi_ssid = doc["wifi_ssid"].as<String>();
    config.wifi_password = doc["wifi_password"].as<String>();

    Serial.println("Configuration loaded:");
    Serial.println("SSID: " + config.wifi_ssid);

    return !config.wifi_ssid.isEmpty();
}

bool SetupServer::saveConfig() {
    Serial.println("Saving configuration...");
    Serial.println("SSID: " + config.wifi_ssid);

    StaticJsonDocument<512> doc;
    doc["wifi_ssid"] = config.wifi_ssid;
    doc["wifi_password"] = config.wifi_password;

    File file = SPIFFS.open(CONFIG_FILE, "w");
    if(!file) {
        Serial.println("Failed to create config file");
        return false;
    }

    if(serializeJson(doc, file) == 0) {
        Serial.println("Failed to write config file");
        file.close();
        return false;
    }

    file.close();
    Serial.println("Configuration saved successfully");
    return true;
}

void SetupServer::handleRoot() {
    server->send(200, "text/html", getSetupPage());
}

void SetupServer::handleSave() {
    if (server->hasArg("ssid") && server->hasArg("password")) {
        config.wifi_ssid = server->arg("ssid");
        config.wifi_password = server->arg("password");

        if (saveConfig()) {
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
        <p>The device will now restart and connect to your WiFi network.</p>
        <div class="urls">
            <p>Once connected, you can visit either:</p>
            <p class="url">http://bindicator.local</p>
            <p>or wait a moment and check your router for the device's IP address</p>
        </div>
        <p><small>(If bindicator.local doesn't work, your device might not support mDNS)</small></p>
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

const char* SetupServer::getSetupPage() {
    static String page;

    String wifiStatus = config.wifi_ssid.isEmpty() ?
        "<span style='color: #f44336;'>Not Configured</span>" :
        "<span style='color: #4CAF50;'>Configured (" + config.wifi_ssid + ")</span>";

    String oauthStatus = oauthHandler.isAuthorized() ?
        "<span style='color: #4CAF50;'>Connected</span>" :
        "<span style='color: #f44336;'>Not Connected</span>";

    page = R"html(
<!DOCTYPE html>
<html>
<head>
    <title>Bindicator Setup</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; }
        .setup-section { margin-bottom: 30px; padding: 20px; border: 1px solid #ccc; border-radius: 5px; }
        h1 { color: #333; }
        label { display: block; margin-bottom: 5px; }
        input[type="text"], input[type="password"] {
            width: 100%;
            padding: 8px;
            margin-bottom: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        button {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        button:hover { background-color: #45a049; }
        .status { margin-top: 10px; }
        .disabled {
            background-color: #cccccc;
            cursor: not-allowed;
        }
    </style>
</head>
<body>
    <h1>Bindicator Setup</h1>

    <div class="setup-section">
        <h2>Step 1: WiFi Setup</h2>
        <form action="/save" method="POST">
            <label for="ssid">WiFi Name:</label>
            <input type="text" id="ssid" name="ssid" value=")html" +
    config.wifi_ssid +
    R"html(">

            <label for="password">WiFi Password:</label>
            <input type="password" id="password" name="password" value=")html" +
    config.wifi_password +
    R"html(">

            <button type="submit">Save WiFi Settings</button>
        </form>
    </div>

    <div class="setup-section">
        <h2>Step 2: Google Calendar Setup</h2>)html" +
    (config.wifi_ssid.isEmpty() ?
        "<p>Please configure WiFi first</p>" :
        "<p>Click below to connect your Google Calendar:</p><a href='/oauth'><button>Connect Google Calendar</button></a>") +
    R"html(
    </div>

    <div class="status">
        <h3>Current Status:</h3>
        <p>WiFi Configuration: )html" + wifiStatus + R"html(</p>
        <p>Google Calendar: )html" + oauthStatus + R"html(</p>
    </div>
</body>
</html>)html";

    return page.c_str();
}

void SetupServer::handleOAuth() {
    if (server->hasArg("code")) {
        String code = server->arg("code");
        String error;

        if (oauthHandler.exchangeAuthCode(code, error)) {
            server->send(200, "text/plain", "OK");
        } else {
            server->send(400, "text/plain", "Failed to exchange auth code: " + error);
        }
    } else {
        server->send(400, "text/plain", "No auth code provided");
    }
}
