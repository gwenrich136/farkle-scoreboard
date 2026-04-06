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
    int scoreToDisplay = state.players[state.currentPlayerIndex].score;
    if (state.currentPlayerScoreMode == ScoreDisplayMode::PENDING) {
        scoreToDisplay += state.atRiskScore;
    }
    displays.scoreDisplay.print_number(scoreToDisplay, ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE);
}

void InGamePhase::updateCompetitionScoreDisplay(const GameState& state, const Displays& displays) {
    int leadingScore = calculateLeadingScore(state);
    displays.scoreDisplay.print_number(leadingScore, ScoreDisplay::DisplayType::COMPETITION_SCORE, state.finalRoundTriggered);
}

void InGamePhase::updateProgressGrid(const GameState& state, const Displays& displays) {
    // Rebuild the m_scores array every frame because animation phases (like FarklingPhase)
    // drain atRiskScore continuously, and atRiskScore is not tracked by scoresVersion.
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
    if (state.players.empty()) return;

    int currentPlayerIdx = state.currentPlayerIndex;
    int leaderIdx = getLeaderIndex(state);

    // Fallback if leader is not found (shouldn't happen if players is not empty)
    if (leaderIdx == -1) leaderIdx = currentPlayerIdx;

    int p1Rank = getPlayerRank(state, currentPlayerIdx);
    int p2Rank = getPlayerRank(state, leaderIdx);

    char p1Place[16];
    char p2Place[16];
    getOrdinalString(p1Rank, p1Place, sizeof(p1Place));
    getOrdinalString(p2Rank, p2Place, sizeof(p2Place));

    displays.textDisplay.printHeadToHeadScreen(
        p1Place,
        &state.players[currentPlayerIdx].name,
        state.players[currentPlayerIdx].hue,
        p2Place,
        &state.players[leaderIdx].name,
        state.players[leaderIdx].hue
    );
}

int InGamePhase::getLeaderIndex(const GameState& state) {
    int maxScore = -1;
    int leaderIdx = -1;
    for (size_t i = 0; i < state.players.size(); ++i) {
        if (state.players[i].score > maxScore) {
            maxScore = state.players[i].score;
            leaderIdx = (int)i;
        }
    }
    return leaderIdx;
}

int InGamePhase::getPlayerRank(const GameState& state, int playerIndex) {
    if (playerIndex < 0 || playerIndex >= (int)state.players.size()) return 1;

    int rank = 1;
    int targetScore = state.players[playerIndex].score;

    for (size_t i = 0; i < state.players.size(); ++i) {
        // If someone has a strictly higher score, they push this player down in rank
        if (state.players[i].score > targetScore) {
            rank++;
        }
    }
    return rank;
}

void InGamePhase::getOrdinalString(int rank, char* buffer, size_t bufferSize) {
    if (bufferSize < 5) return; // need at least "Nth\0"

    const char* suffix = "th";
    int lastDigit = rank % 10;
    int lastTwoDigits = rank % 100;

    if (lastTwoDigits < 11 || lastTwoDigits > 13) {
        if (lastDigit == 1) suffix = "st";
        else if (lastDigit == 2) suffix = "nd";
        else if (lastDigit == 3) suffix = "rd";
    }

    snprintf(buffer, bufferSize, "%d%s", rank, suffix);
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
