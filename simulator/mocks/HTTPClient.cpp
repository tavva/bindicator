// ABOUTME: HTTPClient mock for simulator
// ABOUTME: Routes requests to mock JSON files or real libcurl (future)

#include "HTTPClient.h"
#include <cstdlib>
#include <fstream>
#include <sstream>

HTTPClient::HTTPClient() : responseCode(0), useMockMode(true) {
    const char* mockMode = getenv("SIMULATOR_MOCK");
    if (mockMode && std::string(mockMode) == "0") {
        useMockMode = false;
    }
}

HTTPClient::~HTTPClient() {
    end();
}

bool HTTPClient::begin(const String& url) {
    currentUrl = url;
    return true;
}

void HTTPClient::addHeader(const String& name, const String& value) {
    // Store headers if needed
}

int HTTPClient::GET() {
    if (useMockMode) {
        if (loadMockResponse()) {
            return responseCode;
        }
        return 404;
    } else {
        // TODO: Implement real HTTP via libcurl
        Serial.println("Real HTTP not yet implemented");
        return 500;
    }
}

int HTTPClient::POST(const String& payload) {
    if (useMockMode) {
        if (loadMockResponse()) {
            return responseCode;
        }
        return 404;
    } else {
        // TODO: Implement real HTTP via libcurl
        Serial.println("Real HTTP not yet implemented");
        return 500;
    }
}

String HTTPClient::getString() {
    return responseBody;
}

void HTTPClient::end() {
    responseBody = "";
    responseCode = 0;
}

bool HTTPClient::loadMockResponse() {
    // Determine mock file from URL
    std::string mockFile;

    if (currentUrl.find("oauth2.googleapis.com/token") != std::string::npos) {
        mockFile = "simulator/mock_responses/oauth/token_success.json";
    } else if (currentUrl.find("calendar/v3/calendars") != std::string::npos) {
        if (currentUrl.find("/events") != std::string::npos) {
            mockFile = "simulator/mock_responses/calendar/events_none.json";
        } else {
            mockFile = "simulator/mock_responses/calendar/calendars_list.json";
        }
    } else {
        Serial.print("No mock response for URL: ");
        Serial.println(currentUrl);
        return false;
    }

    std::ifstream file(mockFile);
    if (!file.is_open()) {
        Serial.print("Mock file not found: ");
        Serial.println(mockFile);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    responseBody = buffer.str();
    responseCode = 200;

    return true;
}
