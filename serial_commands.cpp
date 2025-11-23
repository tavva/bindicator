#include "serial_commands.h"
#include "bindicator.h"
#ifdef SIMULATOR
#include <fstream>
#include "calendar_handler.h"
#endif

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
        } else if (command == "undo_bin") {
            markBinNotTakenOut();
        }
        #ifdef SIMULATOR
        else if (command == "mock_setup") {
            mockSetup();
        } else if (command.find("mock_bin ") == 0) {
            String binType = command.substr(9);
            mockBinState(binType);
        } else if (command == "check") {
            forceCalendarCheck();
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
    Serial.println("undo_bin    - Mark bin as NOT taken out (revert completion)");
    #ifdef SIMULATOR
    Serial.println("mock_setup  - Simulate completed setup (simulator only)");
    Serial.println("mock_bin <type> - Set bin state: none|recycling|rubbish (simulator only)");
    Serial.println("check       - Force calendar check now (simulator only)");
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

void SerialCommands::markBinNotTakenOut() {
    Serial.println("Reverting bin to not taken out");
    Bindicator::markBinNotTakenOut();
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

void SerialCommands::mockBinState(const String& binType) {
    Serial.print("\n=== Setting mock bin state: ");
    Serial.print(binType);
    Serial.println(" ===");

    // Determine which mock file to use
    std::string mockFile;
    if (binType == "none") {
        mockFile = "mock_responses/calendar/events_none.json";
    } else if (binType == "recycling") {
        mockFile = "mock_responses/calendar/events_recycling.json";
    } else if (binType == "rubbish") {
        mockFile = "mock_responses/calendar/events_rubbish.json";
    } else {
        Serial.println("Invalid bin type! Use: none, recycling, or rubbish");
        return;
    }

    // Copy the selected mock file to be the active one
    std::ifstream src(mockFile, std::ios::binary);
    if (!src.is_open()) {
        Serial.print("Error: Could not open ");
        Serial.println(mockFile.c_str());
        return;
    }

    std::ofstream dst("mock_responses/calendar/events_none.json", std::ios::binary);
    if (!dst.is_open()) {
        Serial.println("Error: Could not write to events_none.json");
        src.close();
        return;
    }

    dst << src.rdbuf();
    src.close();
    dst.close();

    Serial.println("Mock file updated successfully!");
    Serial.println("Calendar will use this state on next check.");
    Serial.println("Type 'check' to force calendar check immediately.");
}

void SerialCommands::forceCalendarCheck() {
    Serial.println("\n=== Forcing calendar check ===");

    // Check if we should perform the calendar check
    if (!Bindicator::shouldCheckCalendar()) {
        Serial.println("Calendar check skipped (conditions not met)");
        return;
    }

    Serial.println("Triggering calendar check...");

    // The calendar task will pick this up
    extern CalendarHandler calendar;
    bool hasRecycling = false;
    bool hasRubbish = false;

    if (calendar.checkForBinEvents(hasRecycling, hasRubbish)) {
        CollectionState state = CollectionState::NO_COLLECTION;

        if (hasRecycling && hasRubbish) {
            Serial.println("Both recycling and rubbish found - using recycling");
            state = CollectionState::RECYCLING_DUE;
        } else if (hasRecycling) {
            state = CollectionState::RECYCLING_DUE;
        } else if (hasRubbish) {
            state = CollectionState::RUBBISH_DUE;
        }

        Serial.printf("Calendar results - recycling: %d, rubbish: %d\n", hasRecycling, hasRubbish);
        Bindicator::updateFromCalendar(state);
        Serial.println("Calendar check complete!");
    } else {
        Serial.println("Calendar check failed");
    }
}
#endif
