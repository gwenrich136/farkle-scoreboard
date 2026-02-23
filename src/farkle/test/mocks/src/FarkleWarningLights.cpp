#include "FarkleWarningLights.h"

FarkleWarningLights::FarkleWarningLights(int pin) {
    captured_state = 0;
    captured_playerCount = 0;
    captured_currentPlayerIndex = 0;
    captured_isBlinking = false;
}

void FarkleWarningLights::begin() {
    // Begin can be empty for the mock
}

void FarkleWarningLights::farkle_state(int state) {
    captured_state = state;
}

void FarkleWarningLights::update(const int* farkleCounts, int playerCount, int currentPlayerIndex, bool isBlinking) {
    captured_farkleCounts.clear();
    for (int i = 0; i < playerCount; ++i) {
        captured_farkleCounts.push_back(farkleCounts[i]);
    }
    captured_playerCount = playerCount;
    captured_currentPlayerIndex = currentPlayerIndex;
    captured_isBlinking = isBlinking;
}

void FarkleWarningLights::alternate(int currentPlayerIndex, int playerCount) {
    // Empty mock implementation, maybe capture something if needed
    captured_currentPlayerIndex = currentPlayerIndex;
    captured_playerCount = playerCount;
}
