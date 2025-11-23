// ABOUTME: Tests for lightweight ArduinoJson simulator mock parsing

#include "mocks/ArduinoJson.h"
#include <cassert>
#include <iostream>

void test_parses_calendar_events() {
    StaticJsonDocument<1024> doc;
    const char* payload =
        "{\n"
        "  \"kind\": \"calendar#events\",\n"
        "  \"items\": [\n"
        "    { \"summary\": \"Bins out (recycling)\" },\n"
        "    { \"summary\": \"Bins out (rubbish)\" }\n"
        "  ]\n"
        "}";

    auto err = deserializeJson(doc, payload);
    assert(!err);

    JsonArray items = doc["items"];
    int count = 0;
    bool sawRecycling = false;
    bool sawRubbish = false;

    for (JsonVariant item : items) {
        String summary = item["summary"].as<String>();
        if (summary.indexOf("(recycling)") >= 0) sawRecycling = true;
        if (summary.indexOf("(rubbish)") >= 0) sawRubbish = true;
        count++;
    }

    assert(count == 2);
    assert(sawRecycling);
    assert(sawRubbish);
    std::cout << "✓ test_parses_calendar_events" << std::endl;
}

void test_parses_nested_event_objects() {
    StaticJsonDocument<2048> doc;
    const char* payload =
        "{\n"
        "  \"kind\": \"calendar#events\",\n"
        "  \"items\": [\n"
        "    {\n"
        "      \"kind\": \"calendar#event\",\n"
        "      \"id\": \"mock_event_1\",\n"
        "      \"summary\": \"Bins out (recycling)\",\n"
        "      \"start\": { \"date\": \"2025-11-21\" },\n"
        "      \"end\": { \"date\": \"2025-11-21\" }\n"
        "    }\n"
        "  ]\n"
        "}";

    auto err = deserializeJson(doc, payload);
    assert(!err);

    JsonArray items = doc["items"];
    assert(items.size() == 1);
    for (JsonVariant item : items) {
        String summary = item["summary"].as<String>();
        assert(summary == "Bins out (recycling)");
        String startDate = item["start"]["date"].as<String>();
        assert(startDate == "2025-11-21");
    }
    String serialized;
    serializeJson(doc, serialized);
    assert(serialized.indexOf("\"summary\":\"Bins out (recycling)\"") >= 0);
    assert(serialized.indexOf("\"date\":\"2025-11-21\"") >= 0);
    std::cout << "✓ test_parses_nested_event_objects" << std::endl;
}

void test_parses_calendar_list_with_ids() {
    StaticJsonDocument<2048> doc;
    const char* payload =
        "{\n"
        "  \"items\": [\n"
        "    { \"id\": \"primary\", \"summary\": \"Main Calendar\" },\n"
        "    { \"id\": \"bins@example.com\", \"summary\": \"Bins\" }\n"
        "  ]\n"
        "}";

    auto err = deserializeJson(doc, payload);
    assert(!err);

    JsonArray items = doc["items"];
    assert(items.size() == 2);

    int index = 0;
    for (JsonVariant item : items) {
        if (index == 0) {
            assert(item["id"].as<String>() == "primary");
            assert(item["summary"].as<String>() == "Main Calendar");
        } else {
            assert(item["id"].as<String>() == "bins@example.com");
            assert(item["summary"].as<String>() == "Bins");
        }
        index++;
    }

    String serialized;
    serializeJson(doc, serialized);
    assert(serialized.indexOf("\"id\":\"bins@example.com\"") >= 0);
    assert(serialized.indexOf("\"summary\":\"Bins\"") >= 0);
    std::cout << "✓ test_parses_calendar_list_with_ids" << std::endl;
}

int main() {
    test_parses_calendar_events();
    test_parses_nested_event_objects();
    test_parses_calendar_list_with_ids();
    std::cout << "All ArduinoJson tests passed!" << std::endl;
    return 0;
}
