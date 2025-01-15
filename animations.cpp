#include "animations.h"

const uint8_t Animations::exclamation[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,0,0,0,0,0}
};

void Animations::drawError(DisplayHandler& display, uint8_t r, uint8_t g, uint8_t b,
                         uint8_t dot_r, uint8_t dot_g, uint8_t dot_b, uint8_t brightness) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (exclamation[row][col]) {
                if (row == 6) {  // dot
                    display.matrix.setPixelColor(row * 8 + col,
                        display.matrix.Color(
                            (dot_r * brightness) / 64,
                            (dot_g * brightness) / 64,
                            (dot_b * brightness) / 64
                        )
                    );
                } else {  // stroke
                    display.matrix.setPixelColor(row * 8 + col,
                        display.matrix.Color(
                            (r * brightness) / 64,
                            (g * brightness) / 64,
                            (b * brightness) / 64
                        )
                    );
                }
            }
        }
    }
}

void Animations::drawLoading(DisplayHandler& display, int loadingPos) {
    int pos1_row, pos1_col, pos2_row, pos2_col;

    if (loadingPos < 6) {
        pos1_row = 1;
        pos1_col = loadingPos + 1;
    } else if (loadingPos < 12) {
        pos1_row = loadingPos - 4;
        pos1_col = 6;
    } else if (loadingPos < 18) {
        pos1_row = 6;
        pos1_col = 17 - loadingPos;
    } else {
        pos1_row = 23 - loadingPos;
        pos1_col = 1;
    }

    int oppositePos = (loadingPos + 12) % 24;

    if (oppositePos < 6) {
        pos2_row = 1;
        pos2_col = oppositePos + 1;
    } else if (oppositePos < 12) {
        pos2_row = oppositePos - 4;
        pos2_col = 6;
    } else if (oppositePos < 18) {
        pos2_row = 6;
        pos2_col = 17 - oppositePos;
    } else {
        pos2_row = 23 - oppositePos;
        pos2_col = 1;
    }

    display.matrix.setPixelColor(pos1_row * 8 + pos1_col, display.matrix.Color(30, 30, 30));
    display.matrix.setPixelColor(pos2_row * 8 + pos2_col, display.matrix.Color(30, 30, 30));
}

void Animations::drawPulse(DisplayHandler& display, uint8_t r, uint8_t g, uint8_t b,
                         uint8_t brightness) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if(Matrix_Data[row][col] == 1) {
                display.matrix.setPixelColor(row * 8 + col,
                    display.matrix.Color(
                        (r * brightness) / 64,
                        (g * brightness) / 64,
                        (b * brightness) / 64
                    )
                );
            }
        }
    }
}
