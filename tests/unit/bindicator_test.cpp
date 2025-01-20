#include <gtest/gtest.h>
#include "bindicator.h"
#include "tasks.h"

// Mock the command queue
QueueHandle_t commandQueue;

class BindicatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        commandQueue = xQueueCreate(10, sizeof(Command));
        Bindicator::reset();
    }

    void TearDown() override {
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
    Bindicator::handleButtonPress();
    EXPECT_FALSE(Bindicator::shouldCheckCalendar());
}

TEST_F(BindicatorTest, BinTypeManagement) {
    EXPECT_EQ(Bindicator::getCurrentBinType(), BinType::NONE);
}
