#include "LedProgressGrid.h"

LedProgressGrid::LedProgressGrid(uint8_t pin) : player_count(0), was_cleared(false), was_reset(false) {
    // Constructor can be empty for the mock
}

void LedProgressGrid::begin() {
    // Begin can be empty for the mock
}

int LedProgressGrid::addPlayer() {
    return player_count++;
}

void LedProgressGrid::reset() {
    was_reset = true;
    player_count = 0;
}

void LedProgressGrid::clear() {
    was_cleared = true;
}

void LedProgressGrid::update(const std::vector<int>& scores, int currentPlayerIndex, int atRiskScore) {
    captured_scores = scores;
    captured_currentPlayerIndex = currentPlayerIndex;
    captured_atRiskScore = atRiskScore;
}

void LedProgressGrid::displayPlayersPregame(bool isPlayerPending) {
    // Mock implementation for pregame
}
