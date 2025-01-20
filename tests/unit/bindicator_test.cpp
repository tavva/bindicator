#include <gtest/gtest.h>
#include "bindicator.h"
#include "tasks.h"

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

    // Should not check when bin is taken out and before reset time
    Bindicator::setBinType(BinType::RECYCLING);
    Command cmd;
    xQueueReceive(commandQueue, &cmd, 0);  // Clear the RECYCLING command from the queue

    Bindicator::handleButtonPress();
    EXPECT_FALSE(Bindicator::shouldCheckCalendar());

    // Should check and reset when bin is taken out and after reset time
    setMockTime(2024, 3, 21, 15, 0, 0);  // After RESET_HOUR
    EXPECT_TRUE(Bindicator::shouldCheckCalendar());
    EXPECT_FALSE(Bindicator::isBinTakenOut());  // Should be reset
    EXPECT_EQ(Bindicator::getCurrentBinType(), BinType::NONE);  // Should be reset
}

TEST_F(BindicatorTest, BinTypeManagement) {
    EXPECT_EQ(Bindicator::getCurrentBinType(), BinType::NONE);

    Bindicator::setBinType(BinType::RECYCLING);
    EXPECT_EQ(Bindicator::getCurrentBinType(), BinType::RECYCLING);

    Command cmd;
    EXPECT_TRUE(xQueueReceive(commandQueue, &cmd, 0));
    EXPECT_EQ(cmd, CMD_SHOW_RECYCLING);
}
