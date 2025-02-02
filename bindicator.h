#ifndef BINDICATOR_H
#define BINDICATOR_H

#include <Arduino.h>
#include "tasks.h"
#include "bindicator_state.h"

enum class CollectionState {
    NO_COLLECTION,
    RECYCLING_DUE,
    RUBBISH_DUE
};

enum class ErrorType {
    WIFI,
    API
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

    private:
        static BindicatorState state;
        static time_t completedTime;
        static const int RESET_HOUR = 3;

        static void transitionTo(BindicatorState newState);
        static void sendStateCommand(BindicatorState state);
        static void persistState();
        static bool isAfterResetTime();
        static void sendCommand(Command cmd);
};

#endif
