#include "calendar_handler.h"

CalendarHandler::CalendarHandler(OAuthHandler& oauthHandler, const String& calendarId)
    : oauth(oauthHandler), CALENDAR_ID(calendarId) {}

bool CalendarHandler::checkForBinEvents(bool& hasRecycling, bool& hasRubbish) {
    String access_token;
    if (!oauth.getValidToken(access_token)) {
        Serial.println("Failed to get valid token");
        return false;
    }

    HTTPClient http;
    String today = getISODate();
    String url = CALENDAR_API_BASE + CALENDAR_ID + "/events" +
                 "?timeMin=" + today + "T00:00:00Z" +
                 "&timeMax=" + today + "T23:59:59Z" +
                 "&singleEvents=true";
    
    Serial.println("Requesting: " + url);
    
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + access_token);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(8192);
        
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            http.end();
            return false;
        }
        
        hasRecycling = false;
        hasRubbish = false;
        
        JsonArray items = doc["items"];
        for (JsonVariant item : items) {
            String summary = item["summary"].as<String>();
            if (summary.indexOf("(recycling)") >= 0) hasRecycling = true;
            if (summary.indexOf("(rubbish)") >= 0) hasRubbish = true;
        }
        
        http.end();
        return true;
    }
    
    Serial.println("Calendar API Error: " + String(httpResponseCode));
    Serial.println(http.getString());
    http.end();
    return false;
}

String CalendarHandler::getISODate() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return "";
    }
    char dateStr[11];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
    return String(dateStr);
} 