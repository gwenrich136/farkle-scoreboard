#include "GamePhase.h"
#include "GameState.h"
#include <vector>

void InGamePhase::display(const GameState& state, const Displays& displays) {
    // Optimization: Calculate leading score and populate score vector in a single pass
    // to avoid redundant iterations in calculateLeadingScore and updateProgressGrid
    m_scores.clear();
    m_cachedLeadingScore = 0;
    for (const auto& player : state.players) {
        m_scores.push_back(player.score);
        if (player.score > m_cachedLeadingScore) {
            m_cachedLeadingScore = player.score;
        }
    }

    updateScoreDisplays(state, displays);
    updateProgressGrid(state, displays);
    updateWarningLights(state, displays);
    updateTextDisplay(state, displays);
}

void InGamePhase::updateScoreDisplays(const GameState& state, const Displays& displays) {
    updateAtRiskScoreDisplay(state, displays);
    updateCurrentPlayerScoreDisplay(state, displays);
    updateCompetitionScoreDisplay(state, displays);
}

void InGamePhase::updateAtRiskScoreDisplay(const GameState& state, const Displays& displays) {
    if (state.atRiskScore == 0) {
        displays.scoreDisplay.clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
    } else {
        displays.scoreDisplay.print_number(state.atRiskScore, ScoreDisplay::DisplayType::AT_RISK_SCORE);
    }
}

void InGamePhase::updateCurrentPlayerScoreDisplay(const GameState& state, const Displays& displays) {
    displays.scoreDisplay.print_number(state.players[state.currentPlayerIndex].score, ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE);
}

void InGamePhase::updateCompetitionScoreDisplay(const GameState& state, const Displays& displays) {
    // Use cached value calculated in display()
    displays.scoreDisplay.print_number(m_cachedLeadingScore, ScoreDisplay::DisplayType::COMPETITION_SCORE);
}

void InGamePhase::updateProgressGrid(const GameState& state, const Displays& displays) {
    // m_scores is already populated in display()
    displays.grid.update(m_scores, state.currentPlayerIndex, state.atRiskScore);
}

void InGamePhase::updateWarningLights(const GameState& state, const Displays& displays) {
    displays.farkleLights.farkle_state(state.players[state.currentPlayerIndex].farkle_count);
}

void InGamePhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    // Basic turn indicator for now
    displays.oled.print(state.players[state.currentPlayerIndex].name.c_str());
}

int InGamePhase::calculateLeadingScore(const GameState& state) {
    int maxScore = 0;
    for (const auto& player : state.players) {
        if (player.score > maxScore) {
            maxScore = player.score;
        }
    }
    return maxScore;
}

void InGamePhase::endTurn(GameState& state) {
    state.currentPlayerIndex = (state.currentPlayerIndex + 1) % state.players.size();
}
