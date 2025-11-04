#include "mocks/Arduino.h"
#include <cassert>

int main() {
    Serial.begin(115200);
    Serial.println("Testing Serial output");
    Serial.print("Current time: ");
    Serial.println(std::to_string(millis()).c_str());

    delay(1000);
    assert(millis() == 1000);
    Serial.println("✓ Time simulation works");

    return 0;
}
