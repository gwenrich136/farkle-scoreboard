#ifndef MOCK_LED_PROGRESS_GRID_H
#define MOCK_LED_PROGRESS_GRID_H

#include <vector>
#include <cstdint>

class LedProgressGrid {
public:
    // Captured state for inspection by tests
    std::vector<int> captured_scores;
    int captured_currentPlayerIndex;
    int captured_atRiskScore;
    int player_count;
    bool was_cleared;
    bool was_reset;


    LedProgressGrid(uint8_t pin);
    void begin();
    int addPlayer();
    void reset();
    void clear();
    void update(const std::vector<int>& scores, int currentPlayerIndex, int atRiskScore);
    void displayPlayersPregame(bool isPlayerPending);
};

#endif // MOCK_LED_PROGRESS_GRID_H
