#ifndef OAUTH_HANDLER_H
#define OAUTH_HANDLER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class OAuthHandler {
    public:
        OAuthHandler(const String& clientId, const String& clientSecret, const String& refreshToken);
        bool getValidToken(String& token);
    
    private:
        const String CLIENT_ID;
        const String CLIENT_SECRET;
        const String REFRESH_TOKEN;
        const String TOKEN_ENDPOINT = "https://oauth2.googleapis.com/token";
        
        String access_token;
        unsigned long token_expiry = 0;
        
        bool refreshAccessToken();
};

#endif 