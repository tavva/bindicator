#ifndef CALENDAR_HANDLER_H
#define CALENDAR_HANDLER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "oauth_handler.h"
#include "tasks.h"

class CalendarHandler {
    public:
        CalendarHandler(OAuthHandler& oauthHandler, const String& calendarId);
        bool checkForBinEvents(bool& hasRecycling, bool& hasRubbish);

    private:
        OAuthHandler& oauth;
        String access_token;
        const String GOOGLE_CALENDAR_ID;
        const String CALENDAR_API_BASE = "https://www.googleapis.com/calendar/v3/calendars/";

        String getISODate();
        String urlEncode(const String& str);
};

#endif
