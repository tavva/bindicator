#ifndef BINDICATOR_H
#define BINDICATOR_H

#include <Arduino.h>
#include "tasks.h"
#include "bin_type.h"

class Bindicator {
    public:
        static void handleButtonPress();
        static bool shouldCheckCalendar();
        static void setBinType(BinType type);
        static void reset();
        static BinType getCurrentBinType();
        static bool isBinTakenOut();
        static void initializeFromStorage();

    private:
        static BinType currentBinType;
        static bool binTakenOut;
        static const int RESET_HOUR = 3;

        static bool isAfterResetTime();
        static void sendCommand(Command cmd);
};

#endif
