// ABOUTME: ArduinoJson mock implementation for simulator
// ABOUTME: Provides minimal JSON parsing/serialization compatible with firmware

#include "ArduinoJson.h"

// JsonVariant template specializations
template<>
String JsonVariant::as<String>() const {
    return strValue;
}

template<>
const char* JsonVariant::as<const char*>() const {
    return strValue.c_str();
}

template<>
int JsonVariant::as<int>() const {
    return intValue;
}

template<>
JsonArray JsonVariant::as<JsonArray>() const {
    if (isArray && arrayValue) {
        return *arrayValue;
    }
    return JsonArray();
}

JsonVariant::operator JsonArray() const {
    if (isArray && arrayValue) {
        return *arrayValue;
    }
    return JsonArray();
}

JsonVariant JsonVariant::operator[](const char* key) const {
    // Simple mock - just return empty variant
    return JsonVariant();
}

// JsonDocument methods
JsonArray JsonDocument::createNestedArray(const char* key) {
    return JsonArray();
}

template<>
JsonArray JsonDocument::as<JsonArray>() {
    return JsonArray();
}
