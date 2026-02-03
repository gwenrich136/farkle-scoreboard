#include "GamePhase.h"
#include "GameState.h"
#include <vector>

void InGamePhase::display(const GameState& state, const Displays& displays) {
    updateScoreDisplays(state, displays);
    updateProgressGrid(state, displays);
    updateWarningLights(state, displays);
    updateTextDisplay(state, displays);
}

void InGamePhase::updateScoreDisplays(const GameState& state, const Displays& displays) {
    int leadingScore = calculateLeadingScore(state);
    displays.scoreDisplay.print_number(state.atRiskScore, 0);
    displays.scoreDisplay.print_number(state.players[state.currentPlayerIndex].score, 1);
    displays.scoreDisplay.print_number(leadingScore, 2);
}

void InGamePhase::updateProgressGrid(const GameState& state, const Displays& displays) {
    m_scores.clear();
    for (const auto& player : state.players) {
        m_scores.push_back(player.score);
    }
    displays.grid.update(m_scores, state.currentPlayerIndex, state.atRiskScore);
}

void InGamePhase::updateWarningLights(const GameState& state, const Displays& displays) {
    displays.farkleLights.farkle_state(state.players[state.currentPlayerIndex].farkle_count);
}

void InGamePhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    // For V1, we just show the current player's name.
    displays.oled.print(state.players[state.currentPlayerIndex].name.c_str());
}

int InGamePhase::calculateLeadingScore(const GameState& state) {
    int leadingScore = 0;
    for (const auto& player : state.players) {
        if (player.score > leadingScore) {
            leadingScore = player.score;
        }
    }
    return leadingScore;
}

void InGamePhase::endTurn(GameState& state) {
    state.currentPlayerIndex = (state.currentPlayerIndex + 1) % state.players.size();
}
