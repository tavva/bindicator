// ABOUTME: ncurses-based display manager for simulator
// ABOUTME: Provides split-screen view with LED matrix and console output

#pragma once

#include <ncurses.h>
#include <mutex>
#include <cstdint>
#include <string>
#include <queue>

class NcursesDisplay {
private:
    static WINDOW* matrixWin;
    static WINDOW* consoleWin;
    static WINDOW* inputWin;
    static std::mutex displayMutex;
    static bool initialized;
    static int consoleScrollPos;

    // Input handling
    static std::string inputBuffer;
    static std::queue<std::string> commandQueue;
    static std::mutex inputMutex;
    static bool buttonPressed;
    static unsigned long buttonPressStartTime;  // When button press started (millis)
    static unsigned long buttonPressDuration;   // How long to hold button (ms)

public:
    static void init();
    static void cleanup();

    // Matrix rendering (8x8 RGB pixels)
    static void renderMatrix(const uint32_t* pixels, int numPixels);

    // Console output
    static void printConsole(const char* text);

    // Refresh both windows
    static void refresh();

    // Input handling
    static void handleInput();  // Call from main loop
    static bool hasSerialCommand();
    static std::string getSerialCommand();

    // Button simulation
    static bool isButtonPressed();
    static void releaseButton();
    static unsigned long getButtonPressDuration();
};
