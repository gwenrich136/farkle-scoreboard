#ifndef MOCK_SCORE_DISPLAY_H
#define MOCK_SCORE_DISPLAY_H

#include <map>

class ScoreDisplay {
public:
    enum class DisplayType { AT_RISK_SCORE, CURRENT_PLAYER_SCORE, COMPETITION_SCORE };

    std::map<DisplayType, int> captured_numbers;
    std::map<DisplayType, bool> captured_blinks;
    std::map<DisplayType, bool> cleared_displays;
    std::map<DisplayType, int> device_map;

    ScoreDisplay(int dataPin, int clkPin, int csPin);
    void begin();
    void addDisplay(DisplayType type, int deviceIndex);
    void print_number(int number, DisplayType type, bool blink = false);
    void clear(DisplayType type);
};

#endif // MOCK_SCORE_DISPLAY_H
