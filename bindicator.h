#pragma once

#include <Arduino.h>
#include "tasks.h"
#include "bindicator_state.h"
#include "error_type.h"

enum class CollectionState {
    NO_COLLECTION,
    RECYCLING_DUE,
    RUBBISH_DUE
};

class Bindicator {
    public:
        static void handleButtonPress();
        static bool shouldCheckCalendar();
        static void updateFromCalendar(CollectionState collectionState);
        static void initializeFromStorage();
        static void setErrorState(ErrorType errorType);
        static bool isInErrorState();
        static bool isBinTakenOut();
        static void enterSetupMode();
        static void exitSetupMode();
        static bool isInSetupMode();
        static void clearErrorState();

    private:
        static BindicatorState state;
        static time_t completedTime;
        static const int RESET_HOUR = 3;
        static const unsigned long ERROR_RETRY_INTERVAL_MS = 300000;
        static unsigned long lastErrorTime;

        static void transitionTo(BindicatorState newState);
        static void sendStateCommand(BindicatorState state);
        static void persistState();
        static bool isAfterResetTime();
        static void sendCommand(Command cmd);
        static bool shouldRetryAfterError();
};
