#include "bindicator.h"
#include <time.h>
#include "tasks.h"
#include "config_manager.h"

BinType Bindicator::currentBinType = BinType::NONE;
bool Bindicator::binTakenOut = false;

void Bindicator::handleButtonPress() {
    if (!binTakenOut && currentBinType != BinType::NONE) {
        binTakenOut = true;
        ConfigManager::setBinTakenOut(true);
        Command cmd = CMD_SHOW_COMPLETED;
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
    Serial.printf("setBinType: type=%d, currentType=%d, binTakenOut=%d\n",
                 static_cast<int>(type), static_cast<int>(currentBinType), binTakenOut);

    // If the bin type hasn't changed, keep existing state
    if (type == currentBinType) {
        Serial.println("setBinType: Same type, keeping existing state");
        return;
    }

    // Only send command if we're changing to or from a non-NONE type
    if (type != BinType::NONE || currentBinType != BinType::NONE) {
        Command cmd;
        switch(type) {
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

        Serial.printf("setBinType: Sending command %d\n", cmd);
        sendCommand(cmd);
    }

    currentBinType = type;
    ConfigManager::setBinType(type);

    if (binTakenOut) {
        binTakenOut = false;
        ConfigManager::setBinTakenOut(false);
    }
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
    ConfigManager::setBinTakenOut(false);
    ConfigManager::setBinType(BinType::NONE);

    if (currentBinType != BinType::NONE) {
        Command cmd = CMD_SHOW_NEITHER;
        sendCommand(cmd);
    }
}

BinType Bindicator::getCurrentBinType() {
    return currentBinType;
}

bool Bindicator::isBinTakenOut() {
    return binTakenOut;
}

void Bindicator::initializeFromStorage() {
    binTakenOut = ConfigManager::getBinTakenOut();
    currentBinType = ConfigManager::getBinType();
    Serial.printf("initializeFromStorage: binTakenOut=%d, currentBinType=%d\n",
                 binTakenOut, static_cast<int>(currentBinType));

    if (currentBinType == BinType::NONE) {
        Serial.println("initializeFromStorage: No bin type set, skipping command");
        return;
    }

    Command cmd = binTakenOut ? CMD_SHOW_COMPLETED :
        (currentBinType == BinType::RECYCLING ? CMD_SHOW_RECYCLING : CMD_SHOW_RUBBISH);
    Serial.printf("initializeFromStorage: Sending command %d\n", cmd);
    sendCommand(cmd);
}
