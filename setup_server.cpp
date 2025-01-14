#include "setup_server.h"

const char* SetupServer::CONFIG_FILE = "/config.json";

SetupServer::SetupServer() : server(80) {}

void SetupServer::begin() {
    loadConfig();

    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });

    server.begin();
    Serial.println("Setup server started");
}

void SetupServer::handleClient() {
    server.handleClient();
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
    server.send(200, "text/html", getSetupPage());
}

void SetupServer::handleSave() {
    config.wifi_ssid = server.arg("ssid");
    config.wifi_password = server.arg("password");

    if(saveConfig()) {
        String response = R"(
            <!DOCTYPE html>
            <html>
            <head>
                <meta name="viewport" content="width=device-width, initial-scale=1">
                <style>
                    body { font-family: Arial, sans-serif; margin: 20px; }
                    .success { color: #4CAF50; }
                    a { color: #2196F3; text-decoration: none; }
                </style>
            </head>
            <body>
                <h2 class="success">Configuration saved successfully!</h2>
                <p>The device will now restart...</p>
                <script>
                    setTimeout(function() {
                        window.location.href = '/';
                    }, 5000);
                </script>
            </body>
            </html>
        )";
        server.send(200, "text/html", response);

        server.client().flush();
        delay(100);

        ESP.restart();
    } else {
        server.send(500, "text/html", "Failed to save configuration");
    }
}

const char* SetupServer::getSetupPage() {
    static String page = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Bindicator Setup</title>
            <meta name="viewport" content="width=device-width, initial-scale=1">
            <style>
                body {
                    font-family: Arial, sans-serif;
                    margin: 0;
                    padding: 20px;
                    background: #f0f0f0;
                }
                .container {
                    max-width: 400px;
                    margin: 0 auto;
                    background: white;
                    padding: 20px;
                    border-radius: 8px;
                    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
                }
                h1 {
                    color: #333;
                    text-align: center;
                }
                .form-group {
                    margin-bottom: 15px;
                }
                label {
                    display: block;
                    margin-bottom: 5px;
                    color: #666;
                }
                input[type="text"],
                input[type="password"] {
                    width: 100%;
                    padding: 8px;
                    border: 1px solid #ddd;
                    border-radius: 4px;
                    box-sizing: border-box;
                }
                button {
                    background: #4CAF50;
                    color: white;
                    padding: 10px 15px;
                    border: none;
                    border-radius: 4px;
                    cursor: pointer;
                    width: 100%;
                }
                button:hover {
                    background: #45a049;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h1>Bindicator Setup</h1>
                <form action="/save" method="post">
                    <div class="form-group">
                        <label for="ssid">WiFi Name:</label>
                        <input type="text" id="ssid" name="ssid" required>
                    </div>
                    <div class="form-group">
                        <label for="password">WiFi Password:</label>
                        <input type="password" id="password" name="password" required>
                    </div>
                    <button type="submit">Save Configuration</button>
                </form>
            </div>
        </body>
        </html>
    )";
    return page.c_str();
}
