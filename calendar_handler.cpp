#include "secrets.h"
#include "calendar_handler.h"
#include "config_manager.h"
#include "utils.h"

CalendarHandler::CalendarHandler(OAuthHandler& oauthHandler)
    : oauth(oauthHandler) {}

bool CalendarHandler::isBinEvent(const String& summary, bool& isRecycling, bool& isRubbish) const {
    isRecycling = summary.indexOf("(recycling)") >= 0;
    isRubbish = summary.indexOf("(rubbish)") >= 0;
    return isRecycling || isRubbish;
}

bool CalendarHandler::checkForBinEvents(bool& hasRecycling, bool& hasRubbish) {
    String today = getISODate();
    if (today.isEmpty()) {
        Serial.println("Failed to get current date");
        Command cmd = CMD_SHOW_ERROR_OTHER;
        xQueueSend(commandQueue, &cmd, 0);
        return false;
    }

    String timeMin = today + "T00:00:00Z";
    String timeMax = today + "T23:59:59Z";

    Serial.println("Checking calendar for date: " + today);

    if (!oauth.getValidToken(access_token)) {
        Serial.println("Failed to get valid token");
        Command cmd = CMD_SHOW_ERROR_API;
        xQueueSend(commandQueue, &cmd, 0);
        return false;
    }

    HTTPClient http;
    String url = CALENDAR_API_BASE + ConfigManager::getCalendarId() + "/events";
    url += "?timeMin=" + Utils::urlEncode(timeMin);
    url += "&timeMax=" + Utils::urlEncode(timeMax);
    url += "&singleEvents=true";

    Serial.println("Calendar request URL: " + url);

    http.begin(url);
    http.addHeader("Authorization", "Bearer " + access_token);

    int httpResponseCode = http.GET();
    if (httpResponseCode != 200) {
        Serial.println("Calendar API Error: " + String(httpResponseCode));
        Command cmd = CMD_SHOW_ERROR_API;
        xQueueSend(commandQueue, &cmd, 0);
        http.end();
        return false;
    }

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

String CalendarHandler::getISODate(int daysOffset) {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return "";
    }

    timeinfo.tm_mday += daysOffset;
    mktime(&timeinfo);

    char timeStringBuff[11];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d", &timeinfo);
    return String(timeStringBuff);
}

bool CalendarHandler::getAvailableCalendars(JsonDocument& calendars) {
    if (!oauth.getValidToken(access_token)) {
        Serial.println("Failed to get valid token for calendar list request");
        return false;
    }

    HTTPClient http;
    String url = "https://www.googleapis.com/calendar/v3/users/me/calendarList";

    http.begin(url);
    http.addHeader("Authorization", "Bearer " + access_token);

    int httpResponseCode = http.GET();
    if (httpResponseCode != 200) {
        Serial.println("Calendar List API Error: " + String(httpResponseCode));
        http.end();
        return false;
    }

    String payload = http.getString();
    DeserializationError error = deserializeJson(calendars, payload);

    http.end();

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return false;
    }

    return true;
}

bool CalendarHandler::getUpcomingBinDays(JsonDocument& events) {
    if (!oauth.getValidToken(access_token)) {
        Serial.println("Failed to get valid token for upcoming events request");
        return false;
    }

    HTTPClient http;
    String calendarId = ConfigManager::getCalendarId();
    String url = CALENDAR_API_BASE + Utils::urlEncode(calendarId) + "/events";

    String timeMin = getISODate();
    String timeMax = getISODate(DAYS_TO_CHECK_BIN_SCHEDULE);

    if (timeMin.isEmpty() || timeMax.isEmpty()) {
        Serial.println("Failed to get valid time range for upcoming bins");
        return false;
    }

    url += "?timeMin=" + timeMin + "T00:00:00Z";
    url += "&timeMax=" + timeMax + "T23:59:59Z";
    url += "&singleEvents=true";
    url += "&orderBy=startTime";

    http.begin(url);
    http.addHeader("Authorization", "Bearer " + access_token);

    int httpResponseCode = http.GET();
    if (httpResponseCode != 200) {
        Serial.println("Calendar Events API Error: " + String(httpResponseCode));
        String response = http.getString();
        http.end();
        return false;
    }

    String payload = http.getString();
    StaticJsonDocument<4096> allEvents;

    DeserializationError error = deserializeJson(allEvents, payload);

    http.end();

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray filteredItems = events.createNestedArray("items");

    for (JsonVariant event : allEvents["items"].as<JsonArray>()) {
        String summary = event["summary"].as<String>();
        bool isRecycling, isRubbish;
        if (isBinEvent(summary, isRecycling, isRubbish)) {
            filteredItems.add(event);
        }
    }

    return true;
}
