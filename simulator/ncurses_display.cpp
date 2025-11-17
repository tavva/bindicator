// ABOUTME: ncurses-based display manager implementation
// ABOUTME: Manages split-screen terminal UI with thread-safe access

#include "ncurses_display.h"
#include <cstring>
#include <string.h>

WINDOW* NcursesDisplay::matrixWin = nullptr;
WINDOW* NcursesDisplay::consoleWin = nullptr;
WINDOW* NcursesDisplay::inputWin = nullptr;
std::mutex NcursesDisplay::displayMutex;
bool NcursesDisplay::initialized = false;
int NcursesDisplay::consoleScrollPos = 0;

// Input handling
std::string NcursesDisplay::inputBuffer;
std::queue<std::string> NcursesDisplay::commandQueue;
std::mutex NcursesDisplay::inputMutex;
bool NcursesDisplay::buttonPressed = false;
unsigned long NcursesDisplay::buttonPressStartTime = 0;
unsigned long NcursesDisplay::buttonPressDuration = 0;

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
    int inputHeight = 3;   // Input line with borders

    matrixWin = newwin(rows - inputHeight, matrixWidth, 0, 0);
    consoleWin = newwin(rows - inputHeight, cols - matrixWidth, 0, matrixWidth);
    inputWin = newwin(inputHeight, cols, rows - inputHeight, 0);

    // Enable keyboard input (non-blocking)
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    // Draw borders
    box(matrixWin, 0, 0);
    mvwprintw(matrixWin, 0, 2, " LED Matrix ");

    box(consoleWin, 0, 0);
    mvwprintw(consoleWin, 0, 2, " Console Output ");

    box(inputWin, 0, 0);
    mvwprintw(inputWin, 0, 2, " Input: [b]=short press [l]=long press [ENTER]=command [q]=quit ");

    wrefresh(matrixWin);
    wrefresh(consoleWin);
    wrefresh(inputWin);

    initialized = true;
}

void NcursesDisplay::cleanup() {
    std::lock_guard<std::mutex> lock(displayMutex);

    if (!initialized) return;

    if (matrixWin) delwin(matrixWin);
    if (consoleWin) delwin(consoleWin);
    if (inputWin) delwin(inputWin);

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

            if (color != 0) {  // Any non-zero color
                // Determine which color channel is strongest
                int maxChannel = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);

                if (maxChannel < 30) {
                    colorPair = 1; // Very dark, show as black
                } else if (r == g && g == b) {
                    // Grayscale - show as white
                    colorPair = 8;
                } else if (r > 10 && g > 10 && b < 10) {
                    // Yellow (r≈g, low blue) - catches (50,50,0) and similar
                    colorPair = 5;
                } else if (r < 10 && g > 10 && b > 10) {
                    // Cyan (g≈b, low red)
                    colorPair = 6;
                } else if (r > 10 && g < 10 && b > 10) {
                    // Magenta (r≈b, low green)
                    colorPair = 7;
                } else if (r > g && r > b) {
                    colorPair = 2; // Red dominant
                } else if (g > r && g > b) {
                    colorPair = 3; // Green dominant
                } else if (b > r && b > g) {
                    colorPair = 4; // Blue dominant
                } else {
                    colorPair = 8; // Default to white for unclear cases
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
    if (inputWin) wrefresh(inputWin);
}

void NcursesDisplay::handleInput() {
    if (!initialized) return;

    int ch = getch();
    if (ch == ERR) return;  // No input available

    std::lock_guard<std::mutex> lock(inputMutex);

    if (ch == 'q' || ch == 'Q') {
        // Quit signal (handled by main)
        exit(0);
    } else if (ch == 'b' && inputBuffer.empty()) {
        // Short button press simulation (100ms) - only when not typing
        if (!buttonPressed) {
            buttonPressed = true;
            buttonPressStartTime = 0;  // Will be set on first digitalRead
            buttonPressDuration = 100;  // Hold for 100ms
            printConsole("[SIM] Button short press\n");
        }
    } else if ((ch == 'l' || ch == 'L') && inputBuffer.empty()) {
        // Long button press simulation (3.5s for long press detection) - only when not typing
        if (!buttonPressed) {
            buttonPressed = true;
            buttonPressStartTime = 0;  // Will be set on first digitalRead
            buttonPressDuration = 3500;  // Hold for 3.5s
            printConsole("[SIM] Button long press\n");
        }
    } else if (ch == '\n' || ch == KEY_ENTER || ch == 10 || ch == 13) {
        // Submit command
        if (!inputBuffer.empty()) {
            commandQueue.push(inputBuffer);
            printConsole(("> " + inputBuffer + "\n").c_str());
            inputBuffer.clear();
        }
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
        // Backspace
        if (!inputBuffer.empty()) {
            inputBuffer.pop_back();
        }
    } else if (ch >= 32 && ch < 127) {
        // Printable character
        inputBuffer += static_cast<char>(ch);
    }

    // Update input display
    std::lock_guard<std::mutex> displayLock(displayMutex);
    if (inputWin) {
        wmove(inputWin, 1, 1);
        wclrtoeol(inputWin);
        mvwprintw(inputWin, 1, 1, "> %s", inputBuffer.c_str());
        box(inputWin, 0, 0);
        mvwprintw(inputWin, 0, 2, " Input: [b]=short press [l]=long press [ENTER]=command [q]=quit ");
        wrefresh(inputWin);
    }
}

bool NcursesDisplay::hasSerialCommand() {
    std::lock_guard<std::mutex> lock(inputMutex);
    return !commandQueue.empty();
}

std::string NcursesDisplay::getSerialCommand() {
    std::lock_guard<std::mutex> lock(inputMutex);
    if (commandQueue.empty()) return "";

    std::string cmd = commandQueue.front();
    commandQueue.pop();
    return cmd;
}

bool NcursesDisplay::isButtonPressed() {
    std::lock_guard<std::mutex> lock(inputMutex);
    return buttonPressed;
}

void NcursesDisplay::releaseButton() {
    std::lock_guard<std::mutex> lock(inputMutex);
    buttonPressed = false;
}

unsigned long NcursesDisplay::getButtonPressDuration() {
    std::lock_guard<std::mutex> lock(inputMutex);
    return buttonPressDuration;
}
