#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <WiFi.h>
#include "wifi_config.h"
#include "time_manager.h"
#include "oauth_handler.h"
#include "calendar_handler.h"
#include "secrets.h"
#include "display_handler.h"
#include "tasks.h"
#include "setup_server.h"

OAuthHandler oauth(CLIENT_ID, CLIENT_SECRET, REFRESH_TOKEN);
CalendarHandler calendar(oauth, CALENDAR_ID);
DisplayHandler display;
bool hasRecycling, hasRubbish;
SetupServer setupServer;
bool inSetupMode = false;

void startSetupMode() {
    Serial.println("Entering setup mode");
    inSetupMode = true;

    WiFi.softAP("Bindicator Setup", "bindicator123");
    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    setupServer.begin();
}

bool tryWiFiConnection(int maxAttempts = 3) {
    for (int attempt = 1; attempt <= maxAttempts; attempt++) {
        Serial.printf("\nWiFi connection attempt %d of %d\n", attempt, maxAttempts);

        WiFi.begin(setupServer.getWifiSSID().c_str(), setupServer.getWifiPassword().c_str());

        int checks = 0;
        while (WiFi.status() != WL_CONNECTED && checks < 20) {
            delay(500);
            Serial.print(".");
            checks++;
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi connected!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            return true;
        }

        Serial.println("Connection attempt failed");
        WiFi.disconnect(true);
        delay(1000);
    }

    Serial.printf("Failed to connect to WiFi after %d attempts\n", maxAttempts);
    return false;
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting up...");

    if(!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.println("SPIFFS Mounted successfully");

    if (!setupServer.isConfigured()) {
        Serial.println("No configuration found");
        startSetupMode();
    } else {
        Serial.println("Configuration found, starting normal operation");

        if (tryWiFiConnection(3)) {
            startNormalOperation();
        } else {
            Serial.println("Failed to establish WiFi connection");
            startSetupMode();
        }
    }
}

void loop() {
    if (inSetupMode) {
        setupServer.handleClient();
        // Feed the watchdog timer and allow other tasks to run
        delay(10);
    } else {
        // When not in setup mode, delete the loop task
        // as all our work is handled by FreeRTOS tasks
        vTaskDelete(NULL);
    }
}

void startNormalOperation() {
    display.begin();
    display.matrix.clear();
    display.matrix.setBrightness(50);
    memset(Matrix_Data, 1, sizeof(Matrix_Data));

    commandQueue = xQueueCreate(10, sizeof(Command));

    xTaskCreate(
        animationTask,
        "Animation",
        10000,
        NULL,
        3,
        NULL
    );

    xTaskCreate(
        wifiTask,
        "WiFi",
        10000,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        calendarTask,
        "Calendar",
        10000,
        NULL,
        1,
        NULL
    );

    Serial.println("Tasks created");
}
