// ABOUTME: Entry point for Bindicator desktop simulator
// ABOUTME: Provides interactive runtime for testing firmware without hardware

#include <iostream>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include "terminal_display.h"
#include "mocks/Arduino.h"

// Firmware entry points (defined in bindicator.ino)
extern void setup();
extern void loop();

static bool running = true;

static void* loopThread(void* arg) {
    while (running) {
        loop();
        usleep(10000);  // 10ms delay
    }
    return nullptr;
}

int main(int argc, char** argv) {
    std::cout << "Bindicator Simulator v0.1" << std::endl;
    std::cout << "Initializing firmware..." << std::endl;

    // Run firmware setup
    setup();

    std::cout << "\nFirmware initialized. Type 'help' for commands" << std::endl;

    // Start firmware loop in background thread
    pthread_t loop_thread;
    pthread_create(&loop_thread, nullptr, loopThread, nullptr);

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command == "quit" || command == "exit") {
            running = false;
            break;
        } else if (command == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  help  - Show this message" << std::endl;
            std::cout << "  quit  - Exit simulator" << std::endl;
        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    // Wait for loop thread to finish
    pthread_join(loop_thread, nullptr);

    return 0;
}
