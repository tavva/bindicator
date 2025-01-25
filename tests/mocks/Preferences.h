#ifndef PREFERENCES_MOCK_H
#define PREFERENCES_MOCK_H

#include <map>
#include <string>
#include "Arduino.h"  // For String class
#include "bin_type.h"

class Preferences {
public:
    bool begin(const char* name, bool readOnly = false) {
        return true;
    }

    void end() {}

    bool putBool(const char* key, bool value) {
        storage[key] = value ? "true" : "false";
        return true;
    }

    bool getBool(const char* key, bool defaultValue = false) {
        auto it = storage.find(key);
        if (it != storage.end()) {
            return it->second == "true";
        }
        return defaultValue;
    }

    String getString(const char* key, const char* defaultValue = "") {
        auto it = storage.find(key);
        if (it != storage.end()) {
            return String(it->second.c_str());
        }
        return String(defaultValue);
    }

    bool putString(const char* key, const String& value) {
        storage[key] = value.c_str();
        return true;
    }

    void remove(const char* key) {
        storage.erase(key);
    }

    // Add other methods as needed

private:
    std::map<std::string, std::string> storage;
};

#endif
