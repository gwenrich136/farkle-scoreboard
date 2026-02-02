#include "ScoreDisplay.h"

ScoreDisplay::ScoreDisplay(int dataPin, int clkPin, int csPin) {
    // Constructor can be empty for the mock
}

void ScoreDisplay::begin() {
    // Begin can be empty for the mock
}

void ScoreDisplay::print_number(int number, int device_index) {
    captured_numbers[device_index] = number;
}

void ScoreDisplay::clear_display(int device_index) {
    // We can simulate a clear by setting the number to a special value, e.g., -1
    captured_numbers[device_index] = -1;
}
