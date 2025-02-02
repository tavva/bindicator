#define TESTING
#include <gtest/gtest.h>
#include "config_manager.h"

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        ConfigManager::begin();
        Serial.suppressOutput(true);
    }

    void TearDown() override {
        ConfigManager::clearForTesting();
        Serial.suppressOutput(false);
    }
};

TEST_F(ConfigManagerTest, IsConfiguredRequiresWifiCredentials) {
    EXPECT_FALSE(ConfigManager::isConfigured());

    ConfigManager::setWifiCredentials("test_ssid", "");
    EXPECT_FALSE(ConfigManager::isConfigured());

    ConfigManager::setWifiCredentials("", "test_pass");
    EXPECT_FALSE(ConfigManager::isConfigured());

    ConfigManager::setWifiCredentials("test_ssid", "test_pass");
    EXPECT_TRUE(ConfigManager::isConfigured());

    EXPECT_EQ(ConfigManager::getWifiSSID(), "test_ssid");
    EXPECT_EQ(ConfigManager::getWifiPassword(), "test_pass");
}

TEST_F(ConfigManagerTest, WifiCredentialsPersistence) {
    EXPECT_TRUE(ConfigManager::setWifiCredentials("test_ssid", "test_pass"));
    EXPECT_EQ(ConfigManager::getWifiSSID(), "test_ssid");
    EXPECT_EQ(ConfigManager::getWifiPassword(), "test_pass");

    EXPECT_TRUE(ConfigManager::setWifiCredentials("new_ssid", "new_pass"));
    EXPECT_EQ(ConfigManager::getWifiSSID(), "new_ssid");
    EXPECT_EQ(ConfigManager::getWifiPassword(), "new_pass");
}

TEST_F(ConfigManagerTest, ForcedSetupMode) {
    EXPECT_FALSE(ConfigManager::isInForcedSetupMode());

    ConfigManager::setForcedSetupFlag("restart-in-setup-mode");
    EXPECT_TRUE(ConfigManager::isInForcedSetupMode());

    ConfigManager::setForcedSetupFlag("");
    EXPECT_FALSE(ConfigManager::isInForcedSetupMode());

    ConfigManager::setForcedSetupFlag("invalid");
    EXPECT_FALSE(ConfigManager::isInForcedSetupMode());
}

TEST_F(ConfigManagerTest, ProcessSetupFlag) {
    ConfigManager::setForcedSetupFlag("restart-in-setup-mode");
    EXPECT_TRUE(ConfigManager::isInForcedSetupMode());
    ConfigManager::processSetupFlag();
    EXPECT_FALSE(ConfigManager::isInForcedSetupMode());
}

TEST_F(ConfigManagerTest, ProcessSetupFlagOrder) {
    // Set the flag to force setup mode
    ConfigManager::setForcedSetupFlag("restart-in-setup-mode");

    // First verify we're in forced setup mode
    EXPECT_TRUE(ConfigManager::isInForcedSetupMode());

    // Process the flag - this should clear it
    ConfigManager::processSetupFlag();

    // Now we should no longer be in forced setup mode
    EXPECT_FALSE(ConfigManager::isInForcedSetupMode());

    // The flag should be cleared (we can verify this by trying to enter forced setup mode again)
    EXPECT_FALSE(ConfigManager::isInForcedSetupMode());
}
