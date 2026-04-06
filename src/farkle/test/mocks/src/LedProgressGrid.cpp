#include "LedProgressGrid.h"

LedProgressGrid::LedProgressGrid(uint8_t pin) : player_count(0), was_cleared(false), was_reset(false) {
    // Constructor can be empty for the mock
}

void LedProgressGrid::begin() {
    // Begin can be empty for the mock
}

void LedProgressGrid::setTargetScore(int target) {
    captured_targetScore = target;
}

int LedProgressGrid::addPlayer(uint16_t hue) {
    if (isMaxPlayersReached()) {
        return -1;
    }
    return player_count++;
}

bool LedProgressGrid::isMaxPlayersReached() {
    return player_count >= MAX_PLAYERS;
}

void LedProgressGrid::reset() {
    was_reset = true;
    player_count = 0;
}

void LedProgressGrid::clear() {
    was_cleared = true;
}

void LedProgressGrid::update(const int* scores, int playerCount, int currentPlayerIndex, int blinkingScore) {
    captured_scores.clear();
    for (int i = 0; i < playerCount; ++i) {
        captured_scores.push_back(scores[i]);
    }
    captured_currentPlayerIndex = currentPlayerIndex;
    captured_blinkingScore = blinkingScore;
}

void LedProgressGrid::displayPlayersPregame(std::optional<uint16_t> pendingPlayerHue) {
    // Mock implementation for pregame
}
