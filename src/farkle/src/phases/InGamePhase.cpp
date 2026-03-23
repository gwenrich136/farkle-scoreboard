#include "GamePhase.h"
#include "GameState.h"
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

void InGamePhase::updateProgressGrid(const GameState& state, const Displays& displays, bool includeAtRisk, bool blinkAtRisk) {
    if (m_scores.size() != state.players.size() || m_lastScoresVersion != state.scoresVersion) {
        m_scores.clear();
        for (const auto& player : state.players) {
            m_scores.push_back(player.score);
        }
        m_lastScoresVersion = state.scoresVersion;
    }
    displays.grid.update(m_scores.data(), (int)m_scores.size(), state.currentPlayerIndex, state.atRiskScore, includeAtRisk, blinkAtRisk);
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
