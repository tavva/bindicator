// ABOUTME: Entry point for Bindicator desktop simulator
// ABOUTME: Provides interactive runtime for testing firmware without hardware

#include <iostream>
#include <string>
#include "terminal_display.h"
#include "mocks/freertos/FreeRTOS.h"
#include "mocks/freertos/queue.h"

// Firmware globals (needed by firmware code)
QueueHandle_t commandQueue;
bool isBin = false;
uint8_t Matrix_Data[8][8] = {{0}};
uint8_t RGB_Data1[64][3] = {{0}};

int main(int argc, char** argv) {
    std::cout << "Bindicator Simulator v0.1" << std::endl;

    // Initialize firmware globals
    commandQueue = xQueueCreate(10, sizeof(int));
    if (!commandQueue) {
        std::cerr << "Failed to create command queue" << std::endl;
        return 1;
    }

    std::cout << "Type 'help' for commands" << std::endl;

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command == "quit" || command == "exit") {
            break;
        } else if (command == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  help  - Show this message" << std::endl;
            std::cout << "  test  - Test terminal capabilities" << std::endl;
            std::cout << "  quit  - Exit simulator" << std::endl;
        } else if (command == "test") {
            TerminalDisplay display;
            auto caps = display.getCapabilities();
            std::cout << "Terminal capabilities:" << std::endl;
            std::cout << "  UTF-8: " << (caps.hasUTF8 ? "yes" : "no") << std::endl;
            std::cout << "  Color: " << (caps.hasColor ? "yes" : "no") << std::endl;
            std::cout << "  256-color: " << (caps.has256Color ? "yes" : "no") << std::endl;
            std::cout << "  Size: " << caps.width << "x" << caps.height << std::endl;

            display.clear();
            display.moveCursor(1, 1);
            display.setColor(255, 0, 0);
            display.printBlock();
            display.setColor(0, 255, 0);
            display.printBlock();
            display.setColor(0, 0, 255);
            display.printBlock();
            display.resetColor();
            std::cout << " Color test" << std::endl;
        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    return 0;
}
