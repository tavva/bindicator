#pragma once

#include "Arduino.h"
#include <map>
#include <vector>

// Minimal ArduinoJson mock - just enough for our firmware
class JsonVariant {
public:
    JsonVariant() : strValue(""), intValue(0) {}

    template<typename T>
    T as() const;

    const char* as<const char*>() const { return strValue.c_str(); }
    String as<String>() const { return strValue; }
    int as<int>() const { return intValue; }

    void setString(const String& s) { strValue = s; }
    void setInt(int i) { intValue = i; }

    String operator[](const char* key) const {
        return strValue;
    }

private:
    String strValue;
    int intValue;
};

class JsonArray {
public:
    class iterator {
    public:
        iterator(size_t idx, JsonArray* arr) : index(idx), array(arr) {}
        bool operator!=(const iterator& other) const { return index != other.index; }
        iterator& operator++() { ++index; return *this; }
        JsonVariant operator*() const { return array->items[index]; }
    private:
        size_t index;
        JsonArray* array;
    };

    iterator begin() { return iterator(0, this); }
    iterator end() { return iterator(items.size(), this); }

    void add(const JsonVariant& item) { items.push_back(item); }
    size_t size() const { return items.size(); }

private:
    std::vector<JsonVariant> items;
    friend class iterator;
};

class JsonDocument {
public:
    JsonVariant operator[](const char* key) {
        return data[key];
    }

    JsonArray createNestedArray(const char* key) {
        return JsonArray();
    }

    JsonArray as<JsonArray>() {
        return JsonArray();
    }

private:
    std::map<String, JsonVariant> data;
};

class DynamicJsonDocument : public JsonDocument {
public:
    DynamicJsonDocument(size_t capacity) {}
};

template<size_t capacity>
class StaticJsonDocument : public JsonDocument {
public:
    StaticJsonDocument() {}
};

enum class DeserializationError {
    Ok,
    NoMemory,
    InvalidInput
};

inline DeserializationError deserializeJson(JsonDocument& doc, const String& input) {
    // Simple mock - doesn't actually parse JSON
    return DeserializationError::Ok;
}

inline void serializeJson(const JsonDocument& doc, String& output) {
    output = "{}";
}
