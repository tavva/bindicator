#ifndef TIME_MOCK_H
#define TIME_MOCK_H

#include <time.h>

extern time_t mock_current_time;

inline time_t time(time_t* t) {
    if (t != nullptr) {
        *t = mock_current_time;
    }
    return mock_current_time;
}

inline void setMockTime(int year, int month, int day, int hour = 0, int min = 0, int sec = 0) {
    struct tm timeinfo = {};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = min;
    timeinfo.tm_sec = sec;
    mock_current_time = mktime(&timeinfo);
}

inline bool getLocalTime(struct tm* timeinfo) {
    if (mock_current_time == 0) {
        // Default to 2024-01-01 00:00:00 if mock time not set
        setMockTime(2024, 1, 1);
    }
    *timeinfo = *localtime(&mock_current_time);
    return true;
}

// Mock configTime function - no actual NTP setup in tests
inline void configTime(long gmtOffset_sec, int daylightOffset_sec, const char* server) {
    // Just set a default time if none is set
    if (mock_current_time == 0) {
        setMockTime(2024, 1, 1);
    }
}

// Add this to control time in tests
namespace TimeMock {
    inline void setCurrentHour(int hour) {
        // For future use if needed
    }
}

#endif
