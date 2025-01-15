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
        } else if (command == "help") {
            showHelp();
        }
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

void SerialCommands::showHelp() {
    Serial.println("\nAvailable commands:");
    Serial.println("clear - Clear all preferences and restart");
    Serial.println("help  - Show this help message");
}
