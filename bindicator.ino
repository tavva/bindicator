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

        WiFi.begin(setupServer.getWifiSSID().c_str(), setupServer.getWifiPassword().c_str());
        Serial.println("Connecting to WiFi...");

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi connected!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            startNormalOperation();
        } else {
            Serial.println("Failed to connect to WiFi");
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
