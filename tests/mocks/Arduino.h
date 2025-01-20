#ifndef ARDUINO_H
#define ARDUINO_H

#include <string>

class String {
public:
    String() {}
    String(const char* str) : data(str) {}
    String(const std::string& str) : data(str) {}

    char charAt(size_t index) const {
        if (index < data.length()) {
            return data[index];
        }
        return '\0';
    }

    size_t length() const { return data.length(); }

    String& operator+=(char c) {
        data += c;
        return *this;
    }

    String& operator+=(const char* str) {
        data += str;
        return *this;
    }

    const char* c_str() const { return data.c_str(); }

    operator std::string() const { return data; }

    // Add comparison operators
    bool operator==(const String& other) const { return data == other.data; }
    bool operator==(const char* str) const { return data == str; }
    bool operator!=(const String& other) const { return !(*this == other); }
    bool operator!=(const char* str) const { return !(*this == str); }

private:
    std::string data;
};

// Non-member operator for comparing char* with String
inline bool operator==(const char* str, const String& string) {
    return string == str;
}

inline bool operator!=(const char* str, const String& string) {
    return string != str;
}

#endif
