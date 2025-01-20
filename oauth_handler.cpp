#include "oauth_handler.h"
#include <WiFi.h>
#include "utils.h"

OAuthHandler::OAuthHandler(const String& clientId, const String& clientSecret, const String& redirectUri)
    : GOOGLE_CLIENT_ID(clientId),
      GOOGLE_CLIENT_SECRET(clientSecret),
      GOOGLE_REDIRECT_URI(redirectUri) {}

void OAuthHandler::begin(WebServer* server) {
    preferences.begin("oauth", false);
    refresh_token = loadRefreshToken();

    serverAvailable = (server != nullptr);
    if (serverAvailable) {
        server->on("/oauth_callback", HTTP_GET, std::bind(&OAuthHandler::handleOAuthRequest, this, server));
        server->on("/token", HTTP_POST, std::bind(&OAuthHandler::handleTokenRequest, this, server));
    }
}

bool OAuthHandler::isAuthorized() {
    refresh_token = loadRefreshToken();
    return !refresh_token.isEmpty();
}

void OAuthHandler::handleOAuthRequest(WebServer* server) {
    if (!serverAvailable) return;
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
    String state = "device_ip=" + Utils::urlEncode(deviceIP) + "&callback_path=/oauth_callback";

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
    preferences.begin("oauth", false);
    preferences.putString("refresh_token", token);
}

String OAuthHandler::loadRefreshToken() {
    preferences.begin("oauth", false);
    return preferences.getString("refresh_token", "");
}

void OAuthHandler::handleTokenRequest(WebServer* server) {
    if (!serverAvailable) return;
    if (server->hasArg("refresh_token")) {
        String token = server->arg("refresh_token");
        saveRefreshToken(token);
        server->send(200, "text/plain", "OK");
    } else {
        server->send(400, "text/plain", "No refresh token provided");
    }
}
