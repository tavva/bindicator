#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include "time_manager.h"
#include "oauth_handler.h"
#include "calendar_handler.h"
#include "secrets.h"
#include "display_handler.h"
#include "tasks.h"
#include "setup_server.h"
#include "animations.h"
#include "config_manager.h"
#include "serial_commands.h"
#include "button_handler.h"
#include "bindicator.h"
#include "bin_type.h"

OAuthHandler oauth(GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET, GOOGLE_REDIRECT_URI);
CalendarHandler calendar(oauth);
DisplayHandler display;
bool hasRecycling, hasRubbish;
SetupServer setupServer(oauth);
bool inSetupMode = false;

const int BUTTON_PIN = 2;
const int LONG_PRESS_TIME = 3000;
BindicatorButton button(BUTTON_PIN, LONG_PRESS_TIME);

bool setupMDNS() {
    if (!MDNS.begin("bindicator")) {
        Serial.println("Error setting up MDNS responder!");
        return false;
    }
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS responder started at bindicator.local");
    return true;
}

bool tryWiFiConnection(int maxAttempts = 3) {
    for (int attempt = 1; attempt <= maxAttempts; attempt++) {
        Serial.printf("\nWiFi connection attempt %d of %d\n", attempt, maxAttempts);

        WiFi.setHostname("bindicator");
        WiFi.begin(ConfigManager::getWifiSSID().c_str(),
                  ConfigManager::getWifiPassword().c_str());

        int waitCount = 0;
        while (WiFi.status() != WL_CONNECTED && waitCount < 20) {
            delay(500);
            Serial.print(".");
            waitCount++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi connected!");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
            return true;
        }

        Serial.println("\nWiFi connection failed");
        delay(1000);
    }
    return false;
}

void startSetupMode() {
    delay(5000);
    Serial.println("Entering setup mode");
    inSetupMode = true;

    WiFi.mode(WIFI_AP_STA);
    Serial.println("WiFi mode set to AP+STA");
    WiFi.setHostname("bindicator");

    delay(100);

    if (ConfigManager::isConfigured()) {
        Serial.println("Connecting to main network...");
        if (tryWiFiConnection()) {
            Serial.println("\nConnected to main network");
            Serial.print("Station IP Address: ");
            Serial.println(WiFi.localIP());
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        // default IP is 192.168.4.1
        if (!WiFi.softAP("Bindicator Setup", AP_PASSWORD)) {
            Serial.println("AP Start Failed");
            return;
        }
    }

    if (!setupMDNS()) {
        Serial.println("mDNS setup failed");
    }

    setupServer.begin();
    Serial.println("Web server started");
    oauth.begin(setupServer.server);
    Serial.println("OAuth handler initialized");

    Command cmd = CMD_SHOW_SETUP_MODE;
    xQueueSend(commandQueue, &cmd, 0);
}

void startNormalMode() {
    if (!tryWiFiConnection()) {
        Serial.println("Failed to connect to WiFi");
        Command cmd = CMD_SHOW_ERROR_WIFI;
        xQueueSend(commandQueue, &cmd, 0);
        return;
    }

    xTaskCreatePinnedToCore(
        calendarTask,
        "CalendarTask",
        8192,
        NULL,
        1,
        NULL,
        0
    );
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting up...");
    SerialCommands::begin();
    delay(2000);

    ConfigManager::begin();
    display.begin();
    oauth.begin(nullptr);
    commandQueue = xQueueCreate(10, sizeof(Command));

    xTaskCreatePinnedToCore(
        animationTask,
        "AnimationTask",
        8192,
        NULL,
        2,
        NULL,
        1
    );

    if (ConfigManager::isInForcedSetupMode()) {
        Serial.println("Force setup mode enabled");
        startSetupMode();
        ConfigManager::processSetupFlag();
        return;
    }

    if (!ConfigManager::isConfigured() || oauth.loadRefreshToken().isEmpty()) {
        Serial.println("No valid configuration found - entering setup mode");
        startSetupMode();
    } else {
        Serial.println("Configuration loaded - starting normal mode");
        startNormalMode();
    }

    button.begin();

    Bindicator::initializeFromStorage();
}

void loop() {
    SerialCommands::handle();

    if (inSetupMode) {
        setupServer.handleClient();
        delay(50);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
}
