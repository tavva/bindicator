#ifndef SERIAL_COMMANDS_H
#define SERIAL_COMMANDS_H

#include <Arduino.h>
#include "config_manager.h"

class SerialCommands {
    public:
        static void begin();
        static void handle();

    private:
        static void clearAllPreferences();
        static void showHelp();
        static void showPreferences();
        static void printNamespace(const char* name);
};

#endif
