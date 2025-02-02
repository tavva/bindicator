#include "bindicator.h"
#include <time.h>
#include "tasks.h"
#include "config_manager.h"

BindicatorState Bindicator::state = BindicatorState::LOADING;
time_t Bindicator::completedTime = 0;

void Bindicator::handleButtonPress() {
    if (state == BindicatorState::RECYCLING_DUE || state == BindicatorState::RUBBISH_DUE) {
        transitionTo(BindicatorState::COMPLETED);
    }
}

bool Bindicator::shouldCheckCalendar() {
    if (isInErrorState() || isInSetupMode()) {
        return false;
    }

    if (state == BindicatorState::COMPLETED) {
        if (isAfterResetTime()) {
            transitionTo(BindicatorState::LOADING);
            return true;
        }
        return false;
    }

    return true;
}

void Bindicator::updateFromCalendar(CollectionState collectionState) {
    switch (collectionState) {
        case CollectionState::RECYCLING_DUE:
            transitionTo(BindicatorState::RECYCLING_DUE);
            break;
        case CollectionState::RUBBISH_DUE:
            transitionTo(BindicatorState::RUBBISH_DUE);
            break;
        case CollectionState::NO_COLLECTION:
            transitionTo(BindicatorState::NO_COLLECTION);
            break;
    }
}

void Bindicator::setErrorState(bool isWifiError) {
    transitionTo(isWifiError ? BindicatorState::ERROR_WIFI : BindicatorState::ERROR_API);
}

bool Bindicator::isInErrorState() {
    return state == BindicatorState::ERROR_WIFI || state == BindicatorState::ERROR_API;
}

bool Bindicator::isBinTakenOut() {
    return state == BindicatorState::COMPLETED;
}

void Bindicator::transitionTo(BindicatorState newState) {
    Serial.printf("State transition: %d -> %d\n", static_cast<int>(state), static_cast<int>(newState));

    if (newState == state) return;

    if (state == BindicatorState::COMPLETED) {
        completedTime = 0;
    }

    if (newState == BindicatorState::COMPLETED) {
        completedTime = time(nullptr);
    }

    state = newState;
    persistState();
    sendStateCommand(state);
}

void Bindicator::sendStateCommand(BindicatorState state) {
    Command cmd;
    switch (state) {
        case BindicatorState::NO_COLLECTION:
            cmd = CMD_SHOW_NEITHER;
            break;
        case BindicatorState::RECYCLING_DUE:
            cmd = CMD_SHOW_RECYCLING;
            break;
        case BindicatorState::RUBBISH_DUE:
            cmd = CMD_SHOW_RUBBISH;
            break;
        case BindicatorState::COMPLETED:
            cmd = CMD_SHOW_COMPLETED;
            break;
        case BindicatorState::LOADING:
            cmd = CMD_SHOW_LOADING;
            break;
        case BindicatorState::SETUP:
            cmd = CMD_SHOW_SETUP_MODE;
            break;
        case BindicatorState::ERROR_API:
            cmd = CMD_SHOW_ERROR_API;
            break;
        case BindicatorState::ERROR_WIFI:
            cmd = CMD_SHOW_ERROR_WIFI;
            break;
    }
    sendCommand(cmd);
}

void Bindicator::sendCommand(Command cmd) {
    if (xQueueSend(commandQueue, &cmd, 0) != pdTRUE) {
        Serial.println("Failed to send command to queue!");
    }
}

void Bindicator::persistState() {
    ConfigManager::setState(static_cast<int>(state));
    ConfigManager::setCompletedTime(completedTime);
}

bool Bindicator::isAfterResetTime() {
    if (state != BindicatorState::COMPLETED) return false;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to get local time");
        return false;
    }

    struct tm nextReset;
    struct tm completedTimeInfo;
    localtime_r(&completedTime, &nextReset);
    localtime_r(&completedTime, &completedTimeInfo);
    nextReset.tm_hour = RESET_HOUR;
    nextReset.tm_min = 0;
    nextReset.tm_sec = 0;

    if (completedTimeInfo.tm_hour >= RESET_HOUR) {
        nextReset.tm_mday++;
    }

    time_t nextResetTime = mktime(&nextReset);
    time_t now = time(nullptr);

    return now >= nextResetTime;
}

void Bindicator::initializeFromStorage() {
    state = static_cast<BindicatorState>(ConfigManager::getState());
    completedTime = ConfigManager::getCompletedTime();

    if (state == BindicatorState::COMPLETED && isAfterResetTime()) {
        transitionTo(BindicatorState::LOADING);
    } else {
        sendStateCommand(state);
    }
}

void Bindicator::enterSetupMode() {
    transitionTo(BindicatorState::SETUP);
}

void Bindicator::exitSetupMode() {
    transitionTo(BindicatorState::LOADING);
}

bool Bindicator::isInSetupMode() {
    return state == BindicatorState::SETUP;
}
