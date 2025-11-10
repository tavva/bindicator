// ABOUTME: Entry point for Bindicator desktop simulator
// ABOUTME: Provides interactive runtime for testing firmware without hardware

#include <iostream>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "ncurses_display.h"
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

// Signal handler for clean exit
void signalHandler(int signum) {
    running = false;
}

int main(int argc, char** argv) {
    // Set up signal handlers for clean exit
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Initialize ncurses display
    NcursesDisplay::init();
    NcursesDisplay::printConsole("Bindicator Simulator v0.1\n");
    NcursesDisplay::printConsole("Initializing firmware...\n");

    // Run firmware setup
    setup();

    NcursesDisplay::printConsole("\nFirmware initialized. Press Ctrl+C to exit\n");

    // Start firmware loop in background thread
    pthread_t loop_thread;
    pthread_create(&loop_thread, nullptr, loopThread, nullptr);

    // Main thread just keeps display refreshing
    while (running) {
        NcursesDisplay::refresh();
        usleep(100000);  // 100ms
    }

    // Wait for loop thread to finish
    running = false;
    pthread_join(loop_thread, nullptr);

    // Cleanup
    NcursesDisplay::cleanup();

    return 0;
}
