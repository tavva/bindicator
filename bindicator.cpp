#include "bindicator.h"
#include <time.h>
#include "tasks.h"

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

void Bindicator::sendCommand(Command cmd) {
    if (xQueueSend(commandQueue, &cmd, 0) != pdTRUE) {
        Serial.println("Failed to send command to queue!");
    }
}

void Bindicator::setBinType(BinType type) {
    Command cmd;
    switch (type) {
        case BinType::RECYCLING:
            cmd = CMD_SHOW_RECYCLING;
            break;
        case BinType::RUBBISH:
            cmd = CMD_SHOW_RUBBISH;
            break;
        default:
            cmd = CMD_SHOW_NEITHER;
            break;
    }

    sendCommand(cmd);

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
