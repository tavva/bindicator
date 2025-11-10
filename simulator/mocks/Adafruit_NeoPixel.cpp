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

    // Render matrix at fixed position (lines 4-11)
    for (int y = 0; y < HEIGHT; y++) {
        std::cout << "\033[" << (4 + y) << ";1H"; // Position at row

        for (int x = 0; x < WIDTH; x++) {
            uint32_t color = pixels[y * WIDTH + x];
            uint8_t r = (color >> 16) & 0xFF;
            uint8_t g = (color >> 8) & 0xFF;
            uint8_t b = color & 0xFF;

            if (r > 0 || g > 0 || b > 0) {
                std::cout << "\033[48;2;" << (int)r << ";" << (int)g << ";" << (int)b << "m  \033[0m";
            } else {
                std::cout << "\033[48;2;20;20;20m  \033[0m";
            }
        }
        std::cout << "\033[K"; // Clear to end of line
    }

    std::cout.flush();
}
