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

OAuthHandler oauth(GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET, GOOGLE_REDIRECT_URI);
CalendarHandler calendar(oauth, GOOGLE_CALENDAR_ID);
DisplayHandler display;
bool hasRecycling, hasRubbish;
SetupServer setupServer(oauth);
bool inSetupMode = false;

bool setupMDNS() {
    if (!MDNS.begin("bindicator")) {
        Serial.println("Error setting up MDNS responder!");
        return false;
    }
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS responder started at bindicator.local");
    return true;
}

void startSetupMode() {
    Serial.println("Entering setup mode");
    inSetupMode = true;

    WiFi.mode(WIFI_AP_STA);
    Serial.println("WiFi mode set to AP+STA");
    delay(100);

    // default IP is 192.168.4.1
    if (!WiFi.softAP("Bindicator Setup", AP_PASSWORD)) {
        Serial.println("AP Start Failed");
        return;
    }

    if (setupServer.isConfigured()) {
        Serial.println("Connecting to main network...");
        if (tryWiFiConnection()) {
            Serial.println("\nConnected to main network");
            Serial.print("Station IP Address: ");
            Serial.println(WiFi.localIP());
        }
    }

    if (!setupMDNS()) {
        Serial.println("mDNS setup failed");
    }

    setupServer.begin();
    Serial.println("Web server started");
    oauth.begin(setupServer.server);
    Serial.println("OAuth handler initialized");
}

bool tryWiFiConnection(int maxAttempts = 3) {
    for (int attempt = 1; attempt <= maxAttempts; attempt++) {
        Serial.printf("\nWiFi connection attempt %d of %d\n", attempt, maxAttempts);

        WiFi.begin(setupServer.getWifiSSID().c_str(), setupServer.getWifiPassword().c_str());

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
        WiFi.disconnect();
        delay(1000);
    }
    return false;
}

void startNormalMode() {
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

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.println("SPIFFS Mounted Successfully");

    display.begin();

    WiFi.mode(WIFI_STA);
    delay(500);

    setupServer.begin();
    delay(100);

    oauth.begin(setupServer.server);

    if (!setupServer.isConfigured() || !oauth.isAuthorized()) {
        startSetupMode();
    } else {
        startNormalMode();
    }
}

void loop() {
    if (inSetupMode) {
        setupServer.handleClient();

        if (setupServer.isConfigured() && oauth.isAuthorized()) {
            Serial.println("Setup complete, restarting...");
            delay(1000);
            ESP.restart(); // to enter normal operation mode
        }

        yield();
    }
}
