#ifndef OAUTH_HANDLER_H
#define OAUTH_HANDLER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <Preferences.h>

class OAuthHandler {
    public:
        OAuthHandler(const String& clientId, const String& clientSecret, const String& redirectUri);
        void begin(WebServer* server);
        void handleOAuthRequest(WebServer* server);
        bool exchangeAuthCode(const String& code, String& error);
        String getAccessToken() { return access_token; }
        bool hasValidToken() { return !access_token.isEmpty(); }
        bool isAuthorized();
        bool getValidToken(String& token);
        String getAuthUrl();
        bool hasStoredToken();

    private:
        const String GOOGLE_CLIENT_ID;
        const String GOOGLE_CLIENT_SECRET;
        const String GOOGLE_REDIRECT_URI;
        const String AUTH_ENDPOINT = "https://accounts.google.com/o/oauth2/v2/auth";
        const String TOKEN_ENDPOINT = "https://oauth2.googleapis.com/token";
        const String SCOPE = "https://www.googleapis.com/auth/calendar.readonly";

        String access_token;
        String refresh_token;
        unsigned long token_expiry;
        Preferences preferences;
        bool prefsInitialized = false;

        void handleTokenRequest(WebServer* server);
        bool refreshAccessToken();
        void saveRefreshToken(const String& token);
        String loadRefreshToken();
        String urlEncode(const String& str);
};

#endif
