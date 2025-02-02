#include "bindicator.h"
#include <time.h>
#include "tasks.h"
#include "config_manager.h"

BinType Bindicator::currentBinType = BinType::NONE;
time_t Bindicator::binTakenOutTime = 0;

void Bindicator::handleButtonPress() {
    if (binTakenOutTime == 0 && currentBinType != BinType::NONE) {
        binTakenOutTime = time(nullptr);
        ConfigManager::setBinTakenOutTime(binTakenOutTime);
        Command cmd = CMD_SHOW_COMPLETED;
        xQueueSend(commandQueue, &cmd, 0);
    }
}

bool Bindicator::shouldCheckCalendar() {
    if (binTakenOutTime > 0) {
        if (isAfterResetTime()) {
            reset();
            return true;
        }
        return false;
    }
    return true;
}

void Bindicator::sendCommand(Command cmd) {
    if (xQueueSend(commandQueue, &cmd, 0) != pdTRUE) {
        Serial.println("Failed to send command to queue!");
    }
}

void Bindicator::setBinType(BinType type) {
    Serial.printf("setBinType: type=%d, currentType=%d, binTakenOutTime=%ld\n",
                 static_cast<int>(type), static_cast<int>(currentBinType), binTakenOutTime);

    if (type != currentBinType) {
        binTakenOutTime = 0;
        ConfigManager::setBinTakenOutTime(0);

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
}

bool Bindicator::isAfterResetTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return false;
    }

    // If no bin taken out, just check if we're after reset hour
    if (binTakenOutTime == 0) {
        return timeinfo.tm_hour >= RESET_HOUR;
    }

    struct tm nextReset;
    struct tm binTime;
    localtime_r(&binTakenOutTime, &nextReset);
    localtime_r(&binTakenOutTime, &binTime);
    nextReset.tm_hour = RESET_HOUR;
    nextReset.tm_min = 0;
    nextReset.tm_sec = 0;

    if (binTime.tm_hour >= RESET_HOUR) {
        nextReset.tm_mday++;
    }

    time_t nextResetTime = mktime(&nextReset);
    time_t now = time(nullptr);

    return now >= nextResetTime;
}

void Bindicator::reset() {
    Serial.println("reset: Resetting bindicator state");
    Command cmd = CMD_SHOW_NEITHER;
    sendCommand(cmd);

    binTakenOutTime = 0;
    currentBinType = BinType::NONE;
    ConfigManager::setBinTakenOutTime(0);
    ConfigManager::setBinType(BinType::NONE);
}

BinType Bindicator::getCurrentBinType() {
    return currentBinType;
}

bool Bindicator::isBinTakenOut() {
    return binTakenOutTime > 0;
}

void Bindicator::initializeFromStorage() {
    binTakenOutTime = ConfigManager::getBinTakenOutTime();
    currentBinType = ConfigManager::getBinType();
    Serial.printf("initializeFromStorage: binTakenOut=%d, currentBinType=%d\n",
                 binTakenOutTime, static_cast<int>(currentBinType));

    if (binTakenOutTime > 0 && !isAfterResetTime()) {
        Serial.println("initializeFromStorage: Bin taken out and not after reset, showing completed");
        Command cmd = CMD_SHOW_COMPLETED;
        Serial.printf("initializeFromStorage: Sending command %d\n", cmd);
        sendCommand(cmd);
        return;
    }

    if (binTakenOutTime > 0 || isAfterResetTime()) {
        Serial.println("initializeFromStorage: Will check calendar soon, keeping loading screen");
        return;
    }

    if (currentBinType == BinType::NONE) {
        Serial.println("initializeFromStorage: No bin type set and won't check calendar, showing neither screen");
        Command cmd = CMD_SHOW_NEITHER;
        Serial.printf("initializeFromStorage: Sending command %d\n", cmd);
        sendCommand(cmd);
        return;
    }

    Command cmd = currentBinType == BinType::RECYCLING ? CMD_SHOW_RECYCLING : CMD_SHOW_RUBBISH;
    Serial.printf("initializeFromStorage: Showing current bin type, sending command %d\n", cmd);
    sendCommand(cmd);
}
