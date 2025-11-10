// ABOUTME: ncurses-based display manager for simulator
// ABOUTME: Provides split-screen view with LED matrix and console output

#pragma once

#include <ncurses.h>
#include <mutex>
#include <cstdint>

class NcursesDisplay {
private:
    static WINDOW* matrixWin;
    static WINDOW* consoleWin;
    static std::mutex displayMutex;
    static bool initialized;
    static int consoleScrollPos;

public:
    static void init();
    static void cleanup();

    // Matrix rendering (8x8 RGB pixels)
    static void renderMatrix(const uint32_t* pixels, int numPixels);

    // Console output
    static void printConsole(const char* text);

    // Refresh both windows
    static void refresh();
};
