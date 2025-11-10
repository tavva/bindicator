#pragma once

#include "Arduino.h"

#define HTTP_GET 0
#define HTTP_POST 1

class HTTPClient {
public:
    HTTPClient();
    ~HTTPClient();

    bool begin(const String& url);
    void addHeader(const String& name, const String& value);
    int GET();
    int POST(const String& payload);
    String getString();
    void end();

private:
    String currentUrl;
    String responseBody;
    int responseCode;
    bool useMockMode;

    bool loadMockResponse();
};
