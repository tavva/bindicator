#include <gtest/gtest.h>
#include "bindicator.h"
#include "tasks.h"
#include "bin_type.h"

// Mock the command queue
QueueHandle_t commandQueue;

class BindicatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        Serial.suppressOutput(true);
        commandQueue = xQueueCreate(10, sizeof(Command));
        Bindicator::reset();
        // Set mock time to 2am (before RESET_HOUR of 3am)
        setMockTime(2024, 3, 21, 2, 0, 0);
    }

    void TearDown() override {
        Serial.suppressOutput(false);
        vQueueDelete(commandQueue);
    }
};

TEST_F(BindicatorTest, HandleButtonPress) {
    // Should not mark as complete when no bin is due
    Bindicator::handleButtonPress();
    EXPECT_FALSE(Bindicator::isBinTakenOut());

    // Should mark as complete when bin is due
    Bindicator::setBinType(BinType::RECYCLING);
    Command cmd;
    xQueueReceive(commandQueue, &cmd, 0);  // Clear the RECYCLING command from the queue

    Bindicator::handleButtonPress();
    EXPECT_TRUE(Bindicator::isBinTakenOut());

    // Verify command was sent
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_COMPLETED);
}

TEST_F(BindicatorTest, ShouldCheckCalendar) {
    EXPECT_TRUE(Bindicator::shouldCheckCalendar());

    Bindicator::setBinType(BinType::RECYCLING);
    Command cmd;
    xQueueReceive(commandQueue, &cmd, 0);  // Clear the RECYCLING command from the queue

    Bindicator::handleButtonPress();

    setMockTime(2024, 3, 21, 2, 0, 0);  // Before RESET_HOUR
    EXPECT_FALSE(Bindicator::shouldCheckCalendar());

    setMockTime(2024, 3, 21, 15, 30, 0);  // After RESET_HOUR
    EXPECT_TRUE(Bindicator::shouldCheckCalendar());
}

TEST_F(BindicatorTest, BinTypeManagement) {
    EXPECT_EQ(Bindicator::getCurrentBinType(), BinType::NONE);

    Bindicator::setBinType(BinType::RECYCLING);
    EXPECT_EQ(Bindicator::getCurrentBinType(), BinType::RECYCLING);

    Command cmd;
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RECYCLING);
}

TEST_F(BindicatorTest, BinTakenOutPersistsThroughRestart) {
    // Set initial state
    Bindicator::setBinType(BinType::RECYCLING);
    Command cmd;
    xQueueReceive(commandQueue, &cmd, 0);  // Clear the RECYCLING command
    EXPECT_EQ(cmd, CMD_SHOW_RECYCLING);

    // Mark bin as taken out
    Bindicator::handleButtonPress();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_COMPLETED);
    EXPECT_TRUE(Bindicator::isBinTakenOut());

    // Simulate restart by reinitializing
    Bindicator::initializeFromStorage();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_COMPLETED);
    EXPECT_TRUE(Bindicator::isBinTakenOut());

    // Calendar check confirms same bin - should stay completed
    Bindicator::setBinType(BinType::RECYCLING);
    EXPECT_FALSE(xQueueReceive(commandQueue, &cmd, 0));  // No new command
    EXPECT_TRUE(Bindicator::isBinTakenOut());
}

TEST_F(BindicatorTest, BinTakenOutResetsWithNewBinType) {
    // Set initial state and mark as taken out
    Bindicator::setBinType(BinType::RECYCLING);
    Command cmd;
    xQueueReceive(commandQueue, &cmd, 0);  // Clear the RECYCLING command
    Bindicator::handleButtonPress();
    xQueueReceive(commandQueue, &cmd, 0);  // Clear the COMPLETED command
    EXPECT_TRUE(Bindicator::isBinTakenOut());

    // New bin type should reset taken out status
    Bindicator::setBinType(BinType::RUBBISH);
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RUBBISH);
    EXPECT_FALSE(Bindicator::isBinTakenOut());
}

TEST_F(BindicatorTest, InitializeFromStorageWithNoBinType) {
    // Start with no bin type
    Bindicator::reset();
    Command cmd;

    // Initialize should send CMD_SHOW_NEITHER
    Bindicator::initializeFromStorage();
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_NEITHER);
}
