#include "bindicator.h"
#include <time.h>

BinType Bindicator::currentBinType = BinType::NONE;
bool Bindicator::binTakenOut = false;

void Bindicator::handleButtonPress() {
    if (!binTakenOut && currentBinType != BinType::NONE) {
        binTakenOut = true;
        Command cmd = CMD_SHOW_SETUP_MODE;
        xQueueSend(commandQueue, &cmd, 0);
    }
}

bool Bindicator::shouldCheckCalendar() {
    if (binTakenOut && !isAfterResetTime()) {
        return false;
    }

    if (binTakenOut && isAfterResetTime()) {
        reset();
    }

    return true;
}

void Bindicator::setBinType(BinType type) {
    currentBinType = type;
    binTakenOut = false;
}

bool Bindicator::isAfterResetTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return false;
    }

    return timeinfo.tm_hour >= RESET_HOUR;
}

void Bindicator::reset() {
    binTakenOut = false;
    currentBinType = BinType::NONE;
}

BinType Bindicator::getCurrentBinType() {
    return currentBinType;
}

bool Bindicator::isBinTakenOut() {
    return binTakenOut;
}
