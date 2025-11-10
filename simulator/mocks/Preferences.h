#pragma once

#include "Arduino.h"
#include <map>
#include <string>

class Preferences {
public:
    bool begin(const char* name, bool readOnly = false);
    void end();
    void clear();
    bool remove(const char* key);

    String getString(const char* key, const String& defaultValue = "");
    bool putString(const char* key, const String& value);

    int getInt(const char* key, int defaultValue = 0);
    bool putInt(const char* key, int value);

private:
    std::string currentNamespace;
    std::map<std::string, std::map<std::string, std::string>> storage;

    void loadFromFile();
    void saveToFile();
    std::string getFilename();
};
