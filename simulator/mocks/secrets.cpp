// ABOUTME: Mock secrets for simulator
// ABOUTME: Provides placeholder OAuth credentials and WiFi settings for testing

#include "Arduino.h"

// Define the external constants
extern const char* WIFI_SSID = "simulator-wifi";
extern const char* WIFI_PASSWORD = "simulator-pass";

extern const String GOOGLE_CLIENT_ID = String("mock-client-id");
extern const String GOOGLE_CLIENT_SECRET = String("mock-client-secret");
extern const String GOOGLE_REDIRECT_URI = String("http://localhost:8080/oauth2callback");

extern const String AP_PASSWORD = String("simulator-ap-pass");
