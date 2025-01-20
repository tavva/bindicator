#ifndef TIME_MOCK_H
#define TIME_MOCK_H

#include <time.h>

// Mock time functions
inline bool getLocalTime(struct tm* timeinfo) {
    timeinfo->tm_hour = 0;  // Always return midnight
    return true;
}

// Add this to control time in tests
namespace TimeMock {
    inline void setCurrentHour(int hour) {
        // For future use if needed
    }
}

#endif
