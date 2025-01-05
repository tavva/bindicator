#include "wifi_config.h"
#include "secrets.h"
#include <WiFi.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}
