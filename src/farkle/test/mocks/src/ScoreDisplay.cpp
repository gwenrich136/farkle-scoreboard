#include "ScoreDisplay.h"

ScoreDisplay::ScoreDisplay(int dataPin, int clkPin, int csPin) {}

void ScoreDisplay::begin() {}

void ScoreDisplay::addDisplay(DisplayType type, int deviceIndex) {
    device_map[(int)type] = deviceIndex;
}

void ScoreDisplay::print_number(int number, DisplayType type, bool blink) {
    captured_numbers[(int)type] = number;
    captured_blinks[(int)type] = blink;
    cleared_displays[(int)type] = false;
}

void ScoreDisplay::clear(DisplayType type) {
    cleared_displays[(int)type] = true;
    captured_numbers[(int)type] = 0; // Or some indicator
}
