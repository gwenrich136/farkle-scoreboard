#include "ScoreDisplay.h"

ScoreDisplay::ScoreDisplay(int dataPin, int clkPin, int csPin) {
    // Constructor can be empty for the mock
}

void ScoreDisplay::begin() {
    // Begin can be empty for the mock
}

void ScoreDisplay::print_number(int number, int device_index, bool blink) {
    captured_numbers[device_index] = number;
    captured_blinks[device_index] = blink;
}
