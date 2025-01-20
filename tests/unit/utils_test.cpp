#include <gtest/gtest.h>
#include "utils.h"

TEST(UrlEncodeTest, HandlesEmptyString) {
    EXPECT_EQ(Utils::urlEncode(""), "");
}

TEST(UrlEncodeTest, HandlesPlainText) {
    EXPECT_EQ(Utils::urlEncode("hello"), "hello");
    EXPECT_EQ(Utils::urlEncode("Hello123"), "Hello123");
}

TEST(UrlEncodeTest, HandlesSpaces) {
    EXPECT_EQ(Utils::urlEncode("hello world"), "hello+world");
    EXPECT_EQ(Utils::urlEncode("  "), "++");
}

TEST(UrlEncodeTest, HandlesSpecialCharacters) {
    EXPECT_EQ(Utils::urlEncode("hello!"), "hello%21");
    EXPECT_EQ(Utils::urlEncode("@#$"), "%40%23%24");
    EXPECT_EQ(Utils::urlEncode("100%"), "100%25");
}

TEST(UrlEncodeTest, HandlesComplexStrings) {
    EXPECT_EQ(Utils::urlEncode("Hello, World!"), "Hello%2C+World%21");
    EXPECT_EQ(Utils::urlEncode("email@example.com"), "email%40example%2Ecom");
    EXPECT_EQ(Utils::urlEncode("2024-03-21T00:00:00Z"), "2024%2D03%2D21T00%3A00%3A00Z");
}

TEST(UrlEncodeTest, HandlesUTF8Characters) {
    EXPECT_EQ(Utils::urlEncode("£"), "%C2%A3");
    EXPECT_EQ(Utils::urlEncode("€"), "%E2%82%AC");
}
