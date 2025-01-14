#include "secrets.h"
#include "calendar_handler.h"

CalendarHandler::CalendarHandler(OAuthHandler& oauthHandler, const String& calendarId)
    : oauth(oauthHandler), CALENDAR_ID(calendarId) {}

bool CalendarHandler::checkForBinEvents(bool& hasRecycling, bool& hasRubbish) {
    String today = getISODate();
    if (today.isEmpty()) {
        Serial.println("Failed to get current date");
        return false;
    }

    String timeMin = today + "T00:00:00Z";
    String timeMax = today + "T23:59:59Z";

    Serial.println("Checking calendar for date: " + today);

    if (!oauth.getValidToken(access_token)) {
        Serial.println("Failed to get valid token");
        return false;
    }

    HTTPClient http;
    String url = CALENDAR_API_BASE + CALENDAR_ID + "/events";
    url += "?timeMin=" + urlEncode(timeMin);
    url += "&timeMax=" + urlEncode(timeMax);
    url += "&singleEvents=true";

    Serial.println("Calendar request URL: " + url);

    http.begin(url);
    http.addHeader("Authorization", "Bearer " + access_token);

    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
        String payload = http.getString();

        Serial.println("Calendar API Response:");
        Serial.println(payload);

        StaticJsonDocument<2048> doc;
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
            Serial.println("Found event: " + summary);
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

String CalendarHandler::urlEncode(const String& str) {
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encodedString += '+';
        } else if (isalnum(c)) {
            encodedString += c;
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
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
        }
    }
    return encodedString;
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
