#ifndef MOCK_LED_PROGRESS_GRID_H
#define MOCK_LED_PROGRESS_GRID_H

#include <vector>
#include <cstdint>

#define MAX_PLAYERS 8

class LedProgressGrid {
public:
    // Captured state for inspection by tests
    std::vector<int> captured_scores;
    int captured_currentPlayerIndex;
    int captured_atRiskScore;
    bool captured_includeAtRisk;
    bool captured_blinkAtRisk;
    int player_count;
    bool was_cleared;
    bool was_reset;
    int captured_targetScore;


    LedProgressGrid(uint8_t pin);
    void begin();
    void setTargetScore(int target);
    int addPlayer();
    bool isMaxPlayersReached();
    void reset();
    void clear();
    void update(const int* scores, int playerCount, int currentPlayerIndex, int atRiskScore, bool includeAtRisk = true, bool blinkAtRisk = true);
    void displayPlayersPregame(bool isPlayerPending);
};

#endif // MOCK_LED_PROGRESS_GRID_H
