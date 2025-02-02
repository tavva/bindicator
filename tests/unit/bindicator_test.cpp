#include <gtest/gtest.h>
#include "bindicator.h"
#include "tasks.h"
#include "bin_type.h"
#include "config_manager.h"

// Mock the command queue
QueueHandle_t commandQueue;

class BindicatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        Serial.suppressOutput(true);
        commandQueue = xQueueCreate(10, sizeof(Command));

        ConfigManager::begin();
        ConfigManager::setState(static_cast<int>(BindicatorState::LOADING));
        ConfigManager::setCompletedTime(0);

        Bindicator::initializeFromStorage();

        // before RESET_HOUR of 3am
        setMockTime(2024, 3, 21, 2, 0, 0);
    }

    void TearDown() override {
        Serial.suppressOutput(false);
        vQueueDelete(commandQueue);
    }

    void clearQueue() {
        Command cmd;
        while (xQueueReceive(commandQueue, &cmd, 0)) {}
    }
};

TEST_F(BindicatorTest, HandleButtonPress) {
    Bindicator::handleButtonPress();
    EXPECT_FALSE(Bindicator::isBinTakenOut());

    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    clearQueue();

    Bindicator::handleButtonPress();
    EXPECT_TRUE(Bindicator::isBinTakenOut());

    Command cmd;
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_COMPLETED);
}

TEST_F(BindicatorTest, ShouldCheckCalendar) {
    EXPECT_TRUE(Bindicator::shouldCheckCalendar());

    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    clearQueue();
    Bindicator::handleButtonPress();
    clearQueue();

    setMockTime(2024, 3, 21, 2, 0, 0);  // Before RESET_HOUR
    EXPECT_FALSE(Bindicator::shouldCheckCalendar());

    setMockTime(2024, 3, 21, 15, 30, 0);  // After RESET_HOUR
    EXPECT_TRUE(Bindicator::shouldCheckCalendar());
}

TEST_F(BindicatorTest, CalendarUpdates) {
    Command cmd;
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_LOADING);
    clearQueue();

    Bindicator::updateFromCalendar(CollectionState::NO_COLLECTION);
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_NEITHER);
    clearQueue();

    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RECYCLING);
}

TEST_F(BindicatorTest, BinTakenOutPersistsThroughRestart) {
    Command cmd;
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_LOADING);
    clearQueue();

    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RECYCLING);

    Bindicator::handleButtonPress();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_COMPLETED);
    EXPECT_TRUE(Bindicator::isBinTakenOut());

    // Simulate restart by reinitializing
    clearQueue();
    Bindicator::initializeFromStorage();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_COMPLETED);
    EXPECT_TRUE(Bindicator::isBinTakenOut());

    clearQueue();
    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RECYCLING);
    EXPECT_FALSE(Bindicator::isBinTakenOut());
}

TEST_F(BindicatorTest, BinTakenOutResetsWithNewBinType) {
    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    clearQueue();
    Bindicator::handleButtonPress();
    clearQueue();
    EXPECT_TRUE(Bindicator::isBinTakenOut());

    Bindicator::updateFromCalendar(CollectionState::RUBBISH_DUE);
    Command cmd;
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RUBBISH);
    EXPECT_FALSE(Bindicator::isBinTakenOut());
}

TEST_F(BindicatorTest, InitializeFromStorageWithNoCollection) {
    Bindicator::updateFromCalendar(CollectionState::NO_COLLECTION);
    clearQueue();
    Command cmd;

    Bindicator::initializeFromStorage();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_NEITHER);
}

TEST_F(BindicatorTest, InitializeFromStorageScenarios) {
    Command cmd;

    // Scenario 1: Bin taken out and not after reset time
    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    clearQueue();
    Bindicator::handleButtonPress();
    clearQueue();
    setMockTime(2024, 3, 21, 2, 0, 0);  // Before RESET_HOUR
    Bindicator::initializeFromStorage();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_COMPLETED);

    // Scenario 2: Bin taken out and after reset time
    setMockTime(2024, 3, 21, 15, 30, 0);  // After RESET_HOUR
    Bindicator::initializeFromStorage();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_LOADING);

    // Scenario 3: After reset time (no bin taken out)
    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    clearQueue();
    setMockTime(2024, 3, 21, 15, 30, 0);  // After RESET_HOUR
    Bindicator::initializeFromStorage();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RECYCLING);

    // Scenario 4: No bin taken out, not after reset time, no collection
    Bindicator::updateFromCalendar(CollectionState::NO_COLLECTION);
    clearQueue();
    setMockTime(2024, 3, 21, 2, 0, 0);  // Before RESET_HOUR
    Bindicator::initializeFromStorage();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_NEITHER);

    // Scenario 5: No bin taken out, not after reset time, recycling due
    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    clearQueue();
    setMockTime(2024, 3, 21, 2, 0, 0);  // Before RESET_HOUR
    Bindicator::initializeFromStorage();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RECYCLING);
}

TEST_F(BindicatorTest, SetupModeHandling) {
    Command cmd;
    // Clear initial LOADING state
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_LOADING);
    clearQueue();

    // Should not check calendar in setup mode
    Bindicator::enterSetupMode();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_SETUP_MODE);
    EXPECT_FALSE(Bindicator::shouldCheckCalendar());

    // Should return to loading state when exiting setup
    Bindicator::exitSetupMode();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_LOADING);
    EXPECT_TRUE(Bindicator::shouldCheckCalendar());
}

TEST_F(BindicatorTest, ErrorStateHandling) {
    Command cmd;
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_LOADING);
    clearQueue();

    Bindicator::setErrorState(true);
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_ERROR_WIFI);
    EXPECT_TRUE(Bindicator::isInErrorState());
    EXPECT_FALSE(Bindicator::shouldCheckCalendar());

    Bindicator::setErrorState(false);
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_ERROR_API);
    EXPECT_TRUE(Bindicator::isInErrorState());
    EXPECT_FALSE(Bindicator::shouldCheckCalendar());
}

TEST_F(BindicatorTest, StateTransitions) {
    Command cmd;
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_LOADING);
    clearQueue();

    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RECYCLING);

    Bindicator::updateFromCalendar(CollectionState::RUBBISH_DUE);
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RUBBISH);

    Bindicator::updateFromCalendar(CollectionState::NO_COLLECTION);
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_NEITHER);

    Bindicator::updateFromCalendar(CollectionState::RECYCLING_DUE);
    clearQueue();
    Bindicator::handleButtonPress();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_COMPLETED);

    // Test transition back to loading after reset time
    setMockTime(2024, 3, 21, 4, 0, 0);  // After RESET_HOUR
    Bindicator::initializeFromStorage();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_LOADING);
}
