#ifndef CALENDAR_HANDLER_H
#define CALENDAR_HANDLER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "oauth_handler.h"
#include "tasks.h"

class CalendarHandler {
    public:
        CalendarHandler(OAuthHandler& oauthHandler);
        bool checkForBinEvents(bool& hasRecycling, bool& hasRubbish);
        bool getAvailableCalendars(JsonDocument& calendars);
        bool getUpcomingBinDays(JsonDocument& events);

    private:
        OAuthHandler& oauth;
        String access_token;
        const String CALENDAR_API_BASE = "https://www.googleapis.com/calendar/v3/calendars/";
        static const int DAYS_TO_CHECK_BIN_SCHEDULE = 21;

        String getISODate(int daysOffset = 0);
        String urlEncode(const String& str);
        bool isBinEvent(const String& summary, bool& isRecycling, bool& isRubbish) const;
};

#endif
