#include "GamePhase.h"
#include "GameState.h"
#include "GameConstants.h"
#include <vector>
#include <Arduino.h>

void InGamePhase::display(const GameState& state, const Displays& displays) {
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
    int leadingScore = calculateLeadingScore(state);
    displays.scoreDisplay.print_number(leadingScore, ScoreDisplay::DisplayType::COMPETITION_SCORE, state.finalRoundTriggered);
}

void InGamePhase::updateProgressGrid(const GameState& state, const Displays& displays) {
    // For subclasses where the score calculation logic is dynamic and does not simply increment
    // scoresVersion, we will always recalculate the grid. To be safe, we just rebuild the scores array
    // when finalRoundTriggered is active or when scoresVersion changes, but for simplicity we should
    // probably just recalculate it each frame if we use state.atRiskScore in the grid score for FarklingPhase
    // because atRiskScore doesn't bump scoresVersion.
    // However, the animation runs many frames.

    // Actually, FarklingPhase drains atRiskScore every frame. atRiskScore isn't tracked by scoresVersion.
    // So we must rebuild the m_scores array if it's dependent on atRiskScore or if scoresVersion changed.
    m_scores.clear();
    for (size_t i = 0; i < state.players.size(); ++i) {
        m_scores.push_back(getGridScoreForPlayer(state, i));
    }
    m_lastScoresVersion = state.scoresVersion;

    displays.grid.update(m_scores.data(), (int)m_scores.size(), state.currentPlayerIndex, getBlinkingScore(state));
}

int InGamePhase::getGridScoreForPlayer(const GameState& state, int playerIndex) const {
    return state.players[playerIndex].score;
}

int InGamePhase::getBlinkingScore(const GameState& state) const {
    return 0; // Default behavior is no blinking score
}

void InGamePhase::updateWarningLights(const GameState& state, const Displays& displays) {
    int farkleCounts[MAX_PLAYERS];
    int count = 0;
    for (const auto& player : state.players) {
        if (count < MAX_PLAYERS) {
            farkleCounts[count++] = player.farkle_count;
        }
    }

    // Sync with LedProgressGrid blink logic (500ms half period)
    bool isBlinkOn = (millis() % 1000) > 500;

    // Default behavior for InGamePhase is NO blinking (solid lights)
    // Subclasses like WaitingPhase can override this to pass the current player index
    displays.farkleLights.update(farkleCounts, count, -1, isBlinkOn);
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
