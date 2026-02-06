#include "ScoreDisplay.h"

ScoreDisplay::ScoreDisplay(int dataPin, int clkPin, int csPin) {}

void ScoreDisplay::begin() {}

void ScoreDisplay::addDisplay(DisplayType type, int deviceIndex) {
    device_map[type] = deviceIndex;
}

void ScoreDisplay::print_number(int number, DisplayType type, bool blink) {
    captured_numbers[type] = number;
    captured_blinks[type] = blink;
    cleared_displays[type] = false;
}

void ScoreDisplay::clear(DisplayType type) {
    cleared_displays[type] = true;
    captured_numbers[type] = 0; // Or some indicator
}
