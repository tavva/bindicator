#pragma once

#include <cstdlib>

// Forward declaration to avoid circular dependency
class SerialClass;
extern SerialClass Serial;

class ESPClass {
public:
    void restart();
};

extern ESPClass ESP;
