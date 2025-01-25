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

    // If the bin type hasn't changed and it's already been taken out,
    // don't change the animation
    if (type == currentBinType && binTakenOut) {
        Serial.println("setBinType: Same type and taken out, skipping command");
        return;
    }

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

    currentBinType = type;
    ConfigManager::setBinType(type);
    binTakenOut = false;
    ConfigManager::setBinTakenOut(false);
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

    // Don't send any command if there's no bin type set yet
    if (currentBinType == BinType::NONE) {
        Serial.println("initializeFromStorage: No bin type set, skipping command");
        return;
    }

    // If the bin was taken out, show completion animation
    // Otherwise show the appropriate bin animation
    Command cmd = binTakenOut ? CMD_SHOW_COMPLETED :
        (currentBinType == BinType::RECYCLING ? CMD_SHOW_RECYCLING : CMD_SHOW_RUBBISH);
    Serial.printf("initializeFromStorage: Sending command %d\n", cmd);
    sendCommand(cmd);
}
