#pragma once

#include "Arduino.h"
#include <map>
#include <vector>

class JsonArray;

// Minimal ArduinoJson mock - just enough for our firmware
class JsonVariant {
public:
    JsonVariant() : strValue(""), intValue(0), isArray(false) {}

    // Template method for conversions
    template<typename T>
    T as() const;

    void setString(const String& s) { strValue = s; }
    void setInt(int i) { intValue = i; }
    void setArray(JsonArray* arr) { arrayValue = arr; isArray = true; }

    // Implicit conversion operators
    operator int() const { return intValue; }
    operator String() const { return strValue; }
    operator const char*() const { return strValue.c_str(); }
    operator JsonArray() const;

    JsonVariant operator[](const char* key) const;

private:
    String strValue;
    int intValue;
    bool isArray;
    JsonArray* arrayValue = nullptr;

    friend class JsonArray;
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

    JsonArray createNestedArray(const char* key);

    template<typename T>
    T as();

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

class DeserializationError {
public:
    enum Code {
        Ok,
        NoMemory,
        InvalidInput
    };

    DeserializationError(Code code = Ok) : code_(code) {}

    operator bool() const { return code_ != Ok; }
    const char* c_str() const {
        switch (code_) {
            case Ok: return "Ok";
            case NoMemory: return "NoMemory";
            case InvalidInput: return "InvalidInput";
            default: return "Unknown";
        }
    }

private:
    Code code_;
};

inline DeserializationError deserializeJson(JsonDocument& doc, const String& input) {
    // Simple mock - doesn't actually parse JSON
    return DeserializationError(DeserializationError::Ok);
}

inline void serializeJson(const JsonDocument& doc, String& output) {
    output = "{}";
}
