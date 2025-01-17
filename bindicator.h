#ifndef BINDICATOR_H
#define BINDICATOR_H

#include <Arduino.h>

enum class BinType {
    NONE,
    RECYCLING,
    RUBBISH
};

class Bindicator {
    public:
        static void handleButtonPress();
        static bool shouldCheckCalendar();
        static void setBinType(BinType type);
        static void reset();
        static BinType getCurrentBinType();
        static bool isBinTakenOut();

    private:
        static BinType currentBinType;
        static bool binTakenOut;
        static const int RESET_HOUR = 3;

        static bool isAfterResetTime();
};

#endif
