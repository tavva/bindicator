#include "oauth_handler.h"
#include <WiFi.h>

OAuthHandler::OAuthHandler(const String& clientId, const String& clientSecret, const String& redirectUri)
    : GOOGLE_CLIENT_ID(clientId),
      GOOGLE_CLIENT_SECRET(clientSecret),
      GOOGLE_REDIRECT_URI(redirectUri) {}

void OAuthHandler::begin(WebServer* server) {
    prefsInitialized = preferences.begin("oauth", false);
    refresh_token = loadRefreshToken();
    server->on("/oauth_callback", HTTP_GET, std::bind(&OAuthHandler::handleOAuthRequest, this, server));
    server->on("/token", HTTP_POST, std::bind(&OAuthHandler::handleTokenRequest, this, server));
}

bool OAuthHandler::isAuthorized() {
    refresh_token = loadRefreshToken();
    Serial.print("Checking authorisation. Refresh token: ");
    Serial.println(refresh_token);
    Serial.println(refresh_token.isEmpty() ? "empty" : "present");
    return !refresh_token.isEmpty();
}

void OAuthHandler::handleOAuthRequest(WebServer* server) {
    if (server->hasArg("code")) {
        String error;
        if (exchangeAuthCode(server->arg("code"), error)) {
            server->send(200, "text/html", "<h1>Authorization Successful!</h1><p>You can close this window.</p>");
        } else {
            server->send(400, "text/html", "<h1>Authorization Failed</h1><p>Error: " + error + "</p>");
        }
    } else {
        String html = "<html><head><title>Google Calendar Authorization</title></head><body>";
        html += "<h1>Authorize Bindicator</h1>";
        html += "<p>Click the button below to authorize access to your Google Calendar:</p>";
        html += "<a href='" + getAuthUrl() + "'><button>Authorize</button></a>";
        html += "</body></html>";
        server->send(200, "text/html", html);
    }
}

String OAuthHandler::getAuthUrl() {
    String deviceIP = WiFi.localIP().toString();

    String state = "device_ip=" + urlEncode(deviceIP) + "&callback_path=/oauth_callback";

    String url = AUTH_ENDPOINT;
    url += "?client_id=" + GOOGLE_CLIENT_ID;
    url += "&redirect_uri=" + urlEncode(GOOGLE_REDIRECT_URI);
    url += "&response_type=code";
    url += "&scope=" + urlEncode(SCOPE);
    url += "&access_type=offline";
    url += "&prompt=consent";
    url += "&state=" + urlEncode(state);
    return url;
}

bool OAuthHandler::exchangeAuthCode(const String& code, String& error) {
    HTTPClient http;
    http.begin(TOKEN_ENDPOINT);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String post_data = "code=" + code;
    post_data += "&client_id=" + GOOGLE_CLIENT_ID;
    post_data += "&client_secret=" + GOOGLE_CLIENT_SECRET;
    post_data += "&redirect_uri=" + urlEncode(GOOGLE_REDIRECT_URI);
    post_data += "&grant_type=authorization_code";

    int httpCode = http.POST(post_data);

    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        refresh_token = doc["refresh_token"].as<String>();
        access_token = doc["access_token"].as<String>();
        int expires_in = doc["expires_in"];
        token_expiry = millis() + (expires_in * 1000);

        Serial.println("Received refresh token: " + refresh_token);

        preferences.begin("oauth", false);
        bool saved = preferences.putString("refresh_token", refresh_token);
        Serial.print("Saved refresh token status: ");
        Serial.println(saved ? "SUCCESS" : "FAILED");
        preferences.end();

        http.end();
        return true;
    }

    http.end();
    return false;
}

bool OAuthHandler::getValidToken(String& token) {
    if (access_token != "" && millis() < token_expiry) {
        token = access_token;
        return true;
    }
    if (refreshAccessToken()) {
        token = access_token;
        return true;
    }
    return false;
}

bool OAuthHandler::refreshAccessToken() {
    if (refresh_token.length() == 0) return false;

    HTTPClient http;
    http.begin(TOKEN_ENDPOINT);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String post_data = "client_id=" + GOOGLE_CLIENT_ID;
    post_data += "&client_secret=" + GOOGLE_CLIENT_SECRET;
    post_data += "&refresh_token=" + refresh_token;
    post_data += "&grant_type=refresh_token";

    int httpCode = http.POST(post_data);

    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        access_token = doc["access_token"].as<String>();
        int expires_in = doc["expires_in"];
        token_expiry = millis() + (expires_in * 1000);

        http.end();
        return true;
    }

    http.end();
    return false;
}

void OAuthHandler::saveRefreshToken(const String& token) {
    if (!prefsInitialized) {
        Serial.println("Preferences not initialized");
        return;
    }
    preferences.putString("refresh_token", token);
}

String OAuthHandler::loadRefreshToken() {
    if (!prefsInitialized) {
        Serial.println("ERROR: Preferences not initialized!");
        return "";
    }

    String token = preferences.getString("refresh_token", "");
    Serial.print("Loaded refresh token from preferences: ");
    Serial.println(token.isEmpty() ? "EMPTY" : token);

    return token;
}

String OAuthHandler::urlEncode(const String& str) {
    String encoded = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encoded += '+';
        } else if (isalnum(c)) {
            encoded += c;
        } else {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encoded += '%';
            encoded += code0;
            encoded += code1;
        }
    }
    return encoded;
}

void OAuthHandler::handleTokenRequest(WebServer* server) {
    if (server->hasArg("refresh_token")) {
        String token = server->arg("refresh_token");
        saveRefreshToken(token);
        server->send(200, "text/plain", "OK");
    } else {
        server->send(400, "text/plain", "No refresh token provided");
    }
}

bool OAuthHandler::hasStoredToken() {
    preferences.begin("oauth", true); // Read-only mode
    bool hasToken = preferences.isKey("refresh_token");
    String token = preferences.getString("refresh_token", "");
    preferences.end();

    Serial.print("Checking stored token - exists: ");
    Serial.print(hasToken ? "YES" : "NO");
    Serial.print(", value: ");
    Serial.println(token);

    return hasToken && !token.isEmpty();
}
