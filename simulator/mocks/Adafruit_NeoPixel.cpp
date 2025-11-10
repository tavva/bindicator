// ABOUTME: Adafruit_NeoPixel mock implementation for simulator
// ABOUTME: Renders LED matrix to terminal using ANSI 24-bit color codes

#include "Adafruit_NeoPixel.h"
#include <iostream>

void Adafruit_NeoPixel::show() {
    static bool firstShow = true;

    // Assume 8x8 matrix (64 pixels) for Bindicator
    const int WIDTH = 8;
    const int HEIGHT = 8;

    if (firstShow) {
        // On first show, print the initial matrix without moving cursor
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                uint32_t color = pixels[y * WIDTH + x];
                uint8_t r = (color >> 16) & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t b = color & 0xFF;

                if (r > 0 || g > 0 || b > 0) {
                    std::cout << "\033[48;2;" << (int)r << ";" << (int)g << ";" << (int)b << "m  \033[0m";
                } else {
                    std::cout << "  ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
        std::cout.flush();
        firstShow = false;
        return;
    }

    // Move cursor up to redraw in place
    std::cout << "\033[9A";  // Move up 9 lines (8 for matrix + 1 for spacing)

    // Render each row
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            uint32_t color = pixels[y * WIDTH + x];

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
