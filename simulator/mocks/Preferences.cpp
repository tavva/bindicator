// ABOUTME: ESP32 Preferences mock using JSON file backend
// ABOUTME: Provides persistent storage for WiFi credentials, OAuth tokens, device state

#include "Preferences.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

// Simple JSON parsing/writing (no external lib needed for our simple case)
void Preferences::loadFromFile() {
    std::ifstream file(getFilename());
    if (!file.is_open()) {
        return;  // File doesn't exist yet
    }

    std::string line;
    std::string currentNs;

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Namespace line: [namespace]
        if (line[0] == '[' && line[line.length()-1] == ']') {
            currentNs = line.substr(1, line.length()-2);
            continue;
        }

        // Key-value line: key=value
        size_t pos = line.find('=');
        if (pos != std::string::npos && !currentNs.empty()) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            storage[currentNs][key] = value;
        }
    }
}

void Preferences::saveToFile() {
    std::ofstream file(getFilename());
    if (!file.is_open()) {
        Serial.println("Failed to save preferences");
        return;
    }

    for (const auto& ns : storage) {
        file << "[" << ns.first << "]" << std::endl;
        for (const auto& kv : ns.second) {
            file << kv.first << "=" << kv.second << std::endl;
        }
        file << std::endl;
    }
}

std::string Preferences::getFilename() {
    const char* ephemeral = getenv("SIMULATOR_EPHEMERAL");
    if (ephemeral && std::string(ephemeral) == "1") {
        return "";  // In-memory only
    }
    return "simulator-state.json";
}

bool Preferences::begin(const char* name, bool readOnly) {
    currentNamespace = name;

    if (storage.empty() && !getFilename().empty()) {
        loadFromFile();
    }

    return true;
}

void Preferences::end() {
    // No-op for simulator
}

void Preferences::clear() {
    if (!currentNamespace.empty()) {
        storage[currentNamespace].clear();
        saveToFile();
    }
}

bool Preferences::remove(const char* key) {
    if (currentNamespace.empty()) return false;

    auto it = storage[currentNamespace].find(key);
    if (it != storage[currentNamespace].end()) {
        storage[currentNamespace].erase(it);
        saveToFile();
        return true;
    }
    return false;
}

String Preferences::getString(const char* key, const String& defaultValue) {
    if (currentNamespace.empty()) return defaultValue;

    auto nsIt = storage.find(currentNamespace);
    if (nsIt == storage.end()) return defaultValue;

    auto it = nsIt->second.find(key);
    if (it == nsIt->second.end()) return defaultValue;

    return it->second;
}

bool Preferences::putString(const char* key, const String& value) {
    if (currentNamespace.empty()) return false;

    storage[currentNamespace][key] = value;
    saveToFile();
    return true;
}

int Preferences::getInt(const char* key, int defaultValue) {
    String value = getString(key, "");
    if (value.empty()) return defaultValue;
    return std::stoi(value);
}

bool Preferences::putInt(const char* key, int value) {
    return putString(key, std::to_string(value));
}
