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

void ScoreDisplay::clear(int device_index) {
    captured_numbers[device_index] = -99999; // Represent cleared as a special value in mock
}
