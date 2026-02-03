#include "GamePhase.h"
#include "GameState.h"
#include <vector>

void InGamePhase::display(const GameState& state, const Displays& displays) {
    // 1. Update the 7-segment Score Displays
    // Device 0: At Risk Score
    // Device 1: Current Player's Banked Score
    // Device 2: Leading Score (calculated on the fly)
    int leadingScore = 0;
    for (const auto& player : state.players) {
        if (player.score > leadingScore) {
            leadingScore = player.score;
        }
    }

    displays.scoreDisplay.print_number(state.atRiskScore, 0);
    displays.scoreDisplay.print_number(state.players[state.currentPlayerIndex].score, 1);
    displays.scoreDisplay.print_number(leadingScore, 2);

    // 2. Update the LED Progress Grid
    m_scores.clear();
    for (const auto& player : state.players) {
        m_scores.push_back(player.score);
    }
    displays.grid.update(m_scores, state.currentPlayerIndex, state.atRiskScore);

    // 3. Update the Farkle Warning Lights
    displays.farkleLights.farkle_state(state.players[state.currentPlayerIndex].farkle_count);

    // 4. Update the OLED Text Display
    // For V1, we just show the current player's name.
    displays.oled.print(state.players[state.currentPlayerIndex].name.c_str());
}

void InGamePhase::endTurn(GameState& state) {
    state.currentPlayerIndex = (state.currentPlayerIndex + 1) % state.players.size();
}
