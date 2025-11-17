// ABOUTME: ncurses-based display manager implementation
// ABOUTME: Manages split-screen terminal UI with thread-safe access

#include "ncurses_display.h"
#include <cstring>
#include <string.h>

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
    use_default_colors();
    cbreak();
    noecho();
    curs_set(0);  // Hide cursor

    // Initialize basic color pairs (using standard 8 colors)
    // Black background with colored foreground for "pixels"
    init_pair(1, COLOR_BLACK, COLOR_BLACK);    // Off/dark
    init_pair(2, COLOR_RED, COLOR_RED);        // Red
    init_pair(3, COLOR_GREEN, COLOR_GREEN);    // Green
    init_pair(4, COLOR_BLUE, COLOR_BLUE);      // Blue
    init_pair(5, COLOR_YELLOW, COLOR_YELLOW);  // Yellow
    init_pair(6, COLOR_CYAN, COLOR_CYAN);      // Cyan
    init_pair(7, COLOR_MAGENTA, COLOR_MAGENTA);// Magenta
    init_pair(8, COLOR_WHITE, COLOR_WHITE);    // White/Gray

    // Create windows
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int matrixWidth = 22;  // 8*2 pixels + borders
    matrixWin = newwin(rows, matrixWidth, 0, 0);
    consoleWin = newwin(rows, cols - matrixWidth, 0, matrixWidth);

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

    // Render 8x8 matrix starting at row 2, col 3 (inside border, centered)
    for (int y = 0; y < 8; y++) {
        wmove(matrixWin, y + 2, 3);

        for (int x = 0; x < 8; x++) {
            uint32_t color = pixels[y * 8 + x];

            // Extract RGB components
            uint8_t r = (color >> 16) & 0xFF;
            uint8_t g = (color >> 8) & 0xFF;
            uint8_t b = color & 0xFF;

            // Map to nearest basic color
            int colorPair = 1; // Default black
            int brightness = r + g + b;

            if (brightness > 30) {  // Not completely dark
                // Determine dominant color
                if (r > g && r > b && r > 80) {
                    colorPair = 2; // Red
                } else if (g > r && g > b && g > 80) {
                    colorPair = 3; // Green
                } else if (b > r && b > g && b > 80) {
                    colorPair = 4; // Blue
                } else if (r > 80 && g > 80 && b < 80) {
                    colorPair = 5; // Yellow
                } else if (r < 80 && g > 80 && b > 80) {
                    colorPair = 6; // Cyan
                } else if (r > 80 && g < 80 && b > 80) {
                    colorPair = 7; // Magenta
                } else if (brightness > 150) {
                    colorPair = 8; // White/Gray
                } else {
                    colorPair = 1; // Dark gray
                }
            }

            // Draw pixel as two characters
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

    static int lineCount = 0;
    int rows, cols;
    getmaxyx(consoleWin, rows, cols);

    // Calculate position inside border
    int maxLines = rows - 2;  // Account for top and bottom border
    int currentLine = (lineCount % maxLines) + 1;  // +1 for top border

    // Clear the line and print text
    wmove(consoleWin, currentLine, 1);
    wclrtoeol(consoleWin);

    // Truncate text to fit within window (account for borders)
    int maxWidth = cols - 3;  // -2 for borders, -1 for safety
    char buffer[1024];
    strncpy(buffer, text, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    if (strlen(buffer) > maxWidth) {
        buffer[maxWidth] = '\0';
    }

    mvwprintw(consoleWin, currentLine, 1, "%s", buffer);

    // Redraw border
    box(consoleWin, 0, 0);
    mvwprintw(consoleWin, 0, 2, " Console Output ");

    wrefresh(consoleWin);

    lineCount++;
}

void NcursesDisplay::refresh() {
    if (!initialized) return;

    std::lock_guard<std::mutex> lock(displayMutex);

    if (matrixWin) wrefresh(matrixWin);
    if (consoleWin) wrefresh(consoleWin);
}
