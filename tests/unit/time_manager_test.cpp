#include <gtest/gtest.h>
#include "time_manager.h"

class TimeManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        Serial.suppressOutput(true);
    }

    void TearDown() override {
        Serial.suppressOutput(false);
    }
};

TEST_F(TimeManagerTest, SetupTimeSuccess) {
    EXPECT_TRUE(setupTime());

    struct tm timeinfo;
    EXPECT_TRUE(getLocalTime(&timeinfo));
    EXPECT_GT(timeinfo.tm_year + 1900, 2023); // Should be at least 2024
}

TEST_F(TimeManagerTest, TimeValidation) {
    EXPECT_TRUE(isTimeValid()); // Should be valid as we're after 2024
}
