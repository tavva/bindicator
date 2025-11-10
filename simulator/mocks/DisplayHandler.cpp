// ABOUTME: Display handler mock for simulator
// ABOUTME: Renders 8x8 LED matrix to terminal using ANSI color codes

#include "DisplayHandler.h"
#include "../terminal_display.h"
#include <iostream>

void Adafruit_NeoMatrix::show() {
    static TerminalDisplay display;
    static bool firstShow = true;

    if (firstShow) {
        std::cout << "\n"; // Give space for the matrix
        firstShow = false;
    }

    // Move cursor up to redraw in place
    std::cout << "\033[9A";  // Move up 9 lines (8 for matrix + 1 for spacing)

    // Render each row
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            uint32_t color = pixels[y * 8 + x];

            // Extract RGB components
            uint8_t r = (color >> 16) & 0xFF;
            uint8_t g = (color >> 8) & 0xFF;
            uint8_t b = color & 0xFF;

            // Use ANSI 24-bit color if pixel is lit
            if (r > 0 || g > 0 || b > 0) {
                std::cout << "\033[48;2;" << (int)r << ";" << (int)g << ";" << (int)b << "m  \033[0m";
            } else {
                std::cout << "  "; // Dark pixel
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    std::cout.flush();
}
