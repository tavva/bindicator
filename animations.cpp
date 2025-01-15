#include "animations.h"

const uint8_t Animations::exclamation[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,2,2,0,0,0},
    {0,0,0,0,0,0,0,0}
};

int Animations::brightnessTick = 0;
int Animations::loadingPos = 0;
int Animations::animationCounter = 0;

const Color Animations::ERROR_RED(50, 0, 0);
const Color Animations::ERROR_DOT_BLUE(0, 0, 50);
const Color Animations::ERROR_DOT_YELLOW(50, 50, 0);
const Color Animations::ERROR_DOT_PURPLE(50, 0, 50);
const Color Animations::RECYCLING_GREEN(0, 50, 0);
const Color Animations::RUBBISH_BROWN(20, 40, 0);
const Color Animations::DEFAULT_BLUE(0, 0, 50);
const Color Animations::SETUP_YELLOW(50, 50, 0);
const Color Animations::LOADING_WHITE(50, 50, 50);

Color Animations::prepareColor = LOADING_WHITE;

uint8_t Animations::calculateBrightness() {
    uint8_t brightness = (brightnessTick < 32) ? brightnessTick * 2 : (64 - brightnessTick) * 2;
    brightnessTick = (brightnessTick + 1) % 64;
    return brightness;
}

void Animations::drawError(DisplayHandler& display, ErrorType type) {
    Color dot;
    switch(type) {
        case ErrorType::API:
            dot = ERROR_DOT_BLUE;
            break;
        case ErrorType::WIFI:
            dot = ERROR_DOT_YELLOW;
            break;
        case ErrorType::OTHER:
        default:
            dot = ERROR_DOT_PURPLE;
            break;
    }
    drawError(display, ERROR_RED, dot);
}

void Animations::drawError(DisplayHandler& display, Color stroke, Color dot) {
    uint8_t brightness = calculateBrightness();

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (exclamation[row][col] == 1) {  // stroke
                display.matrix.setPixelColor(row * 8 + col,
                    display.matrix.Color(
                        (stroke.r * brightness) / 64,
                        (stroke.g * brightness) / 64,
                        (stroke.b * brightness) / 64
                    )
                );
            } else if (exclamation[row][col] == 2) {  // dot
                display.matrix.setPixelColor(row * 8 + col,
                    display.matrix.Color(
                        (dot.r * brightness) / 64,
                        (dot.g * brightness) / 64,
                        (dot.b * brightness) / 64
                    )
                );
            }
        }
    }
}

void Animations::updateLoadingPosition() {
    animationCounter++;
    if (animationCounter >= ANIMATION_SPEED) {
        animationCounter = 0;
        loadingPos = (loadingPos + 1) % 24;
    }
}

void Animations::drawPrepare(DisplayHandler& display) {
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

    display.matrix.setPixelColor(pos1_row * 8 + pos1_col, display.matrix.Color(prepareColor.r, prepareColor.g, prepareColor.b));
    display.matrix.setPixelColor(pos2_row * 8 + pos2_col, display.matrix.Color(prepareColor.r, prepareColor.g, prepareColor.b));

    updateLoadingPosition();
}

void Animations::drawLoading(DisplayHandler& display) {
    prepareColor = LOADING_WHITE;
    drawPrepare(display);
}

void Animations::drawSetupMode(DisplayHandler& display) {
    prepareColor = SETUP_YELLOW;
    drawPrepare(display);
}

void Animations::drawPulse(DisplayHandler& display, Color color) {
    uint8_t brightness = calculateBrightness();

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            display.matrix.setPixelColor(row * 8 + col,
                display.matrix.Color(
                    (color.r * brightness) / 64,
                    (color.g * brightness) / 64,
                    (color.b * brightness) / 64
                )
            );
        }
    }
}
