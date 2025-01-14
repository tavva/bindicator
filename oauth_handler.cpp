#include "oauth_handler.h"

OAuthHandler::OAuthHandler(const String& clientId, const String& clientSecret, const String& refreshToken)
    : CLIENT_ID(clientId), CLIENT_SECRET(clientSecret), REFRESH_TOKEN(refreshToken) {}

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
    HTTPClient http;
    http.begin(TOKEN_ENDPOINT);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String post_data = "client_id=" + CLIENT_ID +
                       "&client_secret=" + CLIENT_SECRET +
                       "&refresh_token=" + REFRESH_TOKEN +
                       "&grant_type=refresh_token";

    int httpCode = http.POST(post_data);

    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        access_token = doc["access_token"].as<String>();
        int expires_in = doc["expires_in"];
        token_expiry = millis() + (expires_in * 1000);

        Serial.println("Token refreshed successfully");
        http.end();
        return true;
    }

    Serial.println("Token refresh failed: " + String(httpCode));
    Serial.println(http.getString());
    http.end();
    return false;
}