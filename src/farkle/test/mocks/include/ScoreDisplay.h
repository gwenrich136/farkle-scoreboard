#ifndef MOCK_SCORE_DISPLAY_H
#define MOCK_SCORE_DISPLAY_H

#include <map>

class ScoreDisplay {
public:
    std::map<int, int> captured_numbers;

    ScoreDisplay(int dataPin, int clkPin, int csPin);
    void begin();
    void print_number(int number, int device_index);
    void clear_display(int device_index);
};

#endif // MOCK_SCORE_DISPLAY_H
