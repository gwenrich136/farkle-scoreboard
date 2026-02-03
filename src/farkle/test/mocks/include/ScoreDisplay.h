#ifndef MOCK_SCORE_DISPLAY_H
#define MOCK_SCORE_DISPLAY_H

#include <map>

class ScoreDisplay {
public:
    std::map<int, int> captured_numbers;
    std::map<int, bool> captured_blinks;

    ScoreDisplay(int dataPin, int clkPin, int csPin);
    void begin();
    void print_number(int number, int device_index, bool blink = false);
};

#endif // MOCK_SCORE_DISPLAY_H
