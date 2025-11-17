#include "serial_commands.h"

void SerialCommands::begin() {
    Serial.println("\nType 'help' for available commands");
}

void SerialCommands::handle() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "clear") {
            Serial.println("Clearing all preferences...");
            clearAllPreferences();
        } else if (command == "clear_oauth") {
            Serial.println("Clearing OAuth preferences...");
            clearOAuthPreferences();
        } else if (command == "help") {
            showHelp();
        } else if (command == "prefs") {
            showPreferences();
        } else if (command == "setup") {
            enterSetupMode();
        }
        #ifdef SIMULATOR
        else if (command == "mock_setup") {
            mockSetup();
        }
        #endif
    }
}

void SerialCommands::clearAllPreferences() {
    Preferences systemPrefs;
    systemPrefs.begin("system", false);
    systemPrefs.clear();
    systemPrefs.end();

    Preferences oauthPrefs;
    oauthPrefs.begin("oauth", false);
    oauthPrefs.clear();
    oauthPrefs.end();

    Serial.println("All preferences cleared!");
    ESP.restart();
}

void SerialCommands::clearOAuthPreferences() {
    Preferences oauthPrefs;
    oauthPrefs.begin("oauth", false);
    oauthPrefs.clear();
    oauthPrefs.end();

    Serial.println("OAuth preferences cleared!");
    ESP.restart();
}

void SerialCommands::showHelp() {
    Serial.println("\nAvailable commands:");
    Serial.println("clear       - Clear all preferences and restart");
    Serial.println("clear_oauth - Clear only OAuth preferences and restart");
    Serial.println("prefs       - Show all stored preferences");
    Serial.println("setup       - Enter setup mode");
    #ifdef SIMULATOR
    Serial.println("mock_setup  - Simulate completed setup (simulator only)");
    #endif
    Serial.println("help        - Show this help message");
}

void SerialCommands::printNamespace(const char* name) {
    Preferences prefs;
    prefs.begin(name, true);  // Read-only mode

    Serial.printf("\n=== %s namespace ===\n", name);

    if (strcmp(name, "system") == 0) {
        String ssid = prefs.getString("wifi_ssid", "");
        String pass = prefs.getString("wifi_pass", "");
        Serial.printf("wifi_ssid: %s\n", ssid.isEmpty() ? "(empty)" : ssid.c_str());
        Serial.printf("wifi_pass: %s\n", pass.isEmpty() ? "(empty)" : "(set)");
    }
    else if (strcmp(name, "oauth") == 0) {
        String token = prefs.getString("refresh_token", "");
        Serial.printf("refresh_token: %s\n", token.isEmpty() ? "(empty)" : "(set)");
    }

    prefs.end();
}

void SerialCommands::showPreferences() {
    Serial.println("\nStored Preferences:");
    Serial.println("------------------");

    printNamespace("system");
    printNamespace("oauth");

    Serial.println("------------------");
}

void SerialCommands::enterSetupMode() {
    Serial.println("Entering setup mode...");

    ConfigManager::setForcedSetupFlag("restart-in-setup-mode");

    ESP.restart();
}

#ifdef SIMULATOR
void SerialCommands::mockSetup() {
    Serial.println("\n=== Simulating Setup Completion ===");

    // Set WiFi credentials (from secrets.h)
    Serial.println("Setting WiFi credentials...");
    ConfigManager::setWifiCredentials("simulator-wifi", "simulator-pass");

    // Set a fake OAuth refresh token
    Serial.println("Setting OAuth refresh token...");
    Preferences oauthPrefs;
    oauthPrefs.begin("oauth", false);
    oauthPrefs.putString("refresh_token", "mock_refresh_token_12345");
    oauthPrefs.end();

    // Set a calendar ID
    Serial.println("Setting calendar ID...");
    ConfigManager::setCalendarId("primary");

    // Clear forced setup flag
    Serial.println("Clearing forced setup flag...");
    Preferences systemPrefs;
    systemPrefs.begin("system", false);
    systemPrefs.remove("force_setup");
    systemPrefs.end();

    Serial.println("\n=== Setup simulation complete! ===");
    Serial.println("Restarting to enter normal mode...\n");

    ESP.restart();
}
#endif
