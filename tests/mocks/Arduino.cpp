#include "Arduino.h"

String::String() {}

String::String(const char* str) : data(str) {}

String::String(const std::string& str) : data(str) {}

char String::charAt(size_t index) const {
    if (index < data.length()) {
        return data[index];
    }
    return '\0';
}

size_t String::length() const {
    return data.length();
}

String& String::operator+=(char c) {
    data += c;
    return *this;
}

String& String::operator+=(const char* str) {
    data += str;
    return *this;
}

const char* String::c_str() const {
    return data.c_str();
}

String::operator std::string() const {
    return data;
}
