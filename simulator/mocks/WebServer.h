#pragma once

#include "Arduino.h"
#include <functional>

typedef std::function<void(void)> THandlerFunction;

class WebServer {
public:
    WebServer(int port) : port(port) {}

    void begin() {
        Serial.print("WebServer started on port ");
        Serial.println(port);
    }

    void handleClient() {
        // No-op for simulator
    }

    void on(const String& uri, THandlerFunction handler) {
        // Store handler
    }

    void on(const String& uri, int method, THandlerFunction handler) {
        // Store handler
    }

    bool hasArg(const String& name) { return false; }
    String arg(const String& name) { return ""; }

    void send(int code) {
        Serial.print("HTTP ");
        Serial.println(code);
    }

    void send(int code, const String& contentType, const String& content) {
        Serial.println(content);
    }

    void sendHeader(const String& name, const String& value) {}

private:
    int port;
};
