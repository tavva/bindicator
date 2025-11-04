// ABOUTME: Terminal display handler with capability detection and graceful degradation
// ABOUTME: Provides ANSI color rendering for LED matrix visualization

#include "terminal_display.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/ioctl.h>
#include <unistd.h>

TerminalDisplay::TerminalDisplay() {
    capabilities = detectCapabilities();

    if (!capabilities.hasUTF8) {
        std::cerr << "Warning: UTF-8 not detected, using ASCII fallback" << std::endl;
    }
    if (!capabilities.hasColor) {
        std::cerr << "Warning: Color support not detected" << std::endl;
    }
}

TerminalCapabilities TerminalDisplay::detectCapabilities() {
    TerminalCapabilities caps;

    // Check UTF-8 support via locale
    const char* lang = getenv("LANG");
    caps.hasUTF8 = (lang != nullptr && strstr(lang, "UTF-8") != nullptr);

    // Check color support via TERM
    const char* term = getenv("TERM");
    caps.hasColor = (term != nullptr &&
                     (strstr(term, "color") != nullptr ||
                      strstr(term, "xterm") != nullptr ||
                      strstr(term, "screen") != nullptr));

    // Check 256 color support
    const char* colorterm = getenv("COLORTERM");
    caps.has256Color = (colorterm != nullptr ||
                        (term != nullptr && strstr(term, "256") != nullptr));

    // Get terminal size
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        caps.width = w.ws_col;
        caps.height = w.ws_row;
    } else {
        caps.width = 80;
        caps.height = 24;
    }

    return caps;
}

void TerminalDisplay::clear() {
    if (capabilities.hasColor) {
        std::cout << "\033[2J\033[H";  // Clear screen and move to home
    } else {
        std::cout << "\n\n\n";
    }
}

void TerminalDisplay::moveCursor(int x, int y) {
    if (capabilities.hasColor) {
        std::cout << "\033[" << y << ";" << x << "H";
    }
}

void TerminalDisplay::setColor(int r, int g, int b) {
    if (capabilities.has256Color) {
        // Use 24-bit color
        std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
    } else if (capabilities.hasColor) {
        // Use basic 16 colors (approximate)
        int color = 30;  // Black default
        if (r > 128 && g < 128 && b < 128) color = 31;  // Red
        else if (r < 128 && g > 128 && b < 128) color = 32;  // Green
        else if (r < 128 && g < 128 && b > 128) color = 34;  // Blue
        std::cout << "\033[" << color << "m";
    }
}

void TerminalDisplay::resetColor() {
    if (capabilities.hasColor) {
        std::cout << "\033[0m";
    }
}

void TerminalDisplay::printBlock() {
    if (capabilities.hasUTF8) {
        std::cout << "█";
    } else {
        std::cout << "#";
    }
}
