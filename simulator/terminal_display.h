#pragma once

#include <string>

struct TerminalCapabilities {
    bool hasUTF8;
    bool hasColor;
    bool has256Color;
    int width;
    int height;
};

class TerminalDisplay {
public:
    TerminalDisplay();

    TerminalCapabilities getCapabilities() const { return capabilities; }
    void clear();
    void moveCursor(int x, int y);
    void setColor(int r, int g, int b);
    void resetColor();
    void printBlock();

private:
    TerminalCapabilities detectCapabilities();
    TerminalCapabilities capabilities;
};
