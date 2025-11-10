// ABOUTME: Adafruit_NeoPixel mock implementation for simulator
// ABOUTME: Renders LED matrix to terminal using ANSI 24-bit color codes

#include "Adafruit_NeoPixel.h"
#include <iostream>
#include <mutex>

static std::mutex displayMutex;

void Adafruit_NeoPixel::show() {
    // Assume 8x8 matrix (64 pixels) for Bindicator
    const int WIDTH = 8;
    const int HEIGHT = 8;

    // Lock to prevent interleaved output
    std::lock_guard<std::mutex> lock(displayMutex);

    // Move to matrix area (line 4, after headers)
    for (int y = 0; y < HEIGHT; y++) {
        // Position cursor at start of this matrix row
        std::cout << "\033[" << (4 + y) << ";1H";

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
                std::cout << "\033[48;2;20;20;20m  \033[0m"; // Dark gray for off pixels
            }
        }
    }

    // Move cursor to console area (line 13+) so Serial output goes below matrix
    std::cout << "\033[13;1H";
    std::cout.flush();
}
