// ABOUTME: ncurses-based display manager implementation
// ABOUTME: Manages split-screen terminal UI with thread-safe access

#include "ncurses_display.h"
#include <cstring>

WINDOW* NcursesDisplay::matrixWin = nullptr;
WINDOW* NcursesDisplay::consoleWin = nullptr;
std::mutex NcursesDisplay::displayMutex;
bool NcursesDisplay::initialized = false;
int NcursesDisplay::consoleScrollPos = 0;

void NcursesDisplay::init() {
    std::lock_guard<std::mutex> lock(displayMutex);

    if (initialized) return;

    // Initialize ncurses
    initscr();
    start_color();
    cbreak();
    noecho();
    curs_set(0);  // Hide cursor

    // Initialize color pairs for RGB rendering
    // We'll use color pairs 1-216 for a 6x6x6 RGB cube
    int pairNum = 1;
    for (int r = 0; r < 6; r++) {
        for (int g = 0; g < 6; g++) {
            for (int b = 0; b < 6; b++) {
                int colorR = 1000 * r / 5;
                int colorG = 1000 * g / 5;
                int colorB = 1000 * b / 5;

                init_color(pairNum, colorR, colorG, colorB);
                init_pair(pairNum, pairNum, pairNum);
                pairNum++;
                if (pairNum > 216) goto done_colors;
            }
        }
    }
    done_colors:

    // Create windows
    // Matrix window: 20 cols (for 8*2 + border), full height, on left
    // Console window: remaining width, full height, on right
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int matrixWidth = 22;  // 8*2 pixels + 2 for border + 2 padding
    matrixWin = newwin(rows, matrixWidth, 0, 0);
    consoleWin = newwin(rows, cols - matrixWidth, 0, matrixWidth);

    // Enable scrolling for console window
    scrollok(consoleWin, TRUE);

    // Draw borders
    box(matrixWin, 0, 0);
    mvwprintw(matrixWin, 0, 2, " LED Matrix ");

    box(consoleWin, 0, 0);
    mvwprintw(consoleWin, 0, 2, " Console Output ");

    wrefresh(matrixWin);
    wrefresh(consoleWin);

    initialized = true;
}

void NcursesDisplay::cleanup() {
    std::lock_guard<std::mutex> lock(displayMutex);

    if (!initialized) return;

    if (matrixWin) delwin(matrixWin);
    if (consoleWin) delwin(consoleWin);

    endwin();
    initialized = false;
}

void NcursesDisplay::renderMatrix(const uint32_t* pixels, int numPixels) {
    if (!initialized || !matrixWin) return;

    std::lock_guard<std::mutex> lock(displayMutex);

    // Clear matrix area (inside border)
    for (int y = 1; y <= 8; y++) {
        mvwprintw(matrixWin, y, 1, "                  ");
    }

    // Render 8x8 matrix starting at row 1, col 1 (inside border)
    for (int y = 0; y < 8; y++) {
        wmove(matrixWin, y + 2, 3);  // Start at row 2, col 3 (centered)

        for (int x = 0; x < 8; x++) {
            uint32_t color = pixels[y * 8 + x];

            // Extract RGB components
            uint8_t r = (color >> 16) & 0xFF;
            uint8_t g = (color >> 8) & 0xFF;
            uint8_t b = color & 0xFF;

            // Map to 6x6x6 color cube (216 colors)
            int r6 = (r * 5) / 255;
            int g6 = (g * 5) / 255;
            int b6 = (b * 5) / 255;
            int colorPair = 1 + r6 * 36 + g6 * 6 + b6;

            if (colorPair > 216) colorPair = 216;
            if (colorPair < 1) colorPair = 1;

            // Draw pixel as two characters (roughly square)
            wattron(matrixWin, COLOR_PAIR(colorPair));
            waddstr(matrixWin, "  ");
            wattroff(matrixWin, COLOR_PAIR(colorPair));
        }
    }

    wrefresh(matrixWin);
}

void NcursesDisplay::printConsole(const char* text) {
    if (!initialized || !consoleWin) return;

    std::lock_guard<std::mutex> lock(displayMutex);

    int rows, cols;
    getmaxyx(consoleWin, rows, cols);

    // Print inside the border (row 1 to rows-2, col 1 to cols-2)
    wmove(consoleWin, rows - 2, 1);
    wprintw(consoleWin, "%s", text);
    wscrl(consoleWin, 1);  // Scroll up one line

    // Redraw border since scroll might affect it
    box(consoleWin, 0, 0);
    mvwprintw(consoleWin, 0, 2, " Console Output ");

    wrefresh(consoleWin);
}

void NcursesDisplay::refresh() {
    if (!initialized) return;

    std::lock_guard<std::mutex> lock(displayMutex);

    if (matrixWin) wrefresh(matrixWin);
    if (consoleWin) wrefresh(consoleWin);
}
