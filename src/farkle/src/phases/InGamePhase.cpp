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
    int competitorScore = 0;
    if (!state.rankedPlayerIndices.empty() && state.currentCompetitorRank >= 0 && state.currentCompetitorRank < (int)state.rankedPlayerIndices.size()) {
        int competitorIdx = state.rankedPlayerIndices[state.currentCompetitorRank];
        competitorScore = state.players[competitorIdx].score;
    }
    displays.scoreDisplay.print_number(competitorScore, ScoreDisplay::DisplayType::COMPETITION_SCORE, state.finalRoundTriggered);
}

void InGamePhase::updateProgressGrid(const GameState& state, const Displays& displays) {
    // Rebuild the m_scores array every frame because animation phases (like FarklingPhase)
    // drain atRiskScore continuously, and atRiskScore is not tracked by scoresVersion.
    m_scoresCount = 0;
    for (size_t i = 0; i < state.players.size() && m_scoresCount < MAX_PLAYERS; ++i) {
        m_scores[m_scoresCount++] = getGridScoreForPlayer(state, i);
    }
    m_lastScoresVersion = state.scoresVersion;

    displays.grid.update(m_scores, m_scoresCount, state.currentPlayerIndex, getBlinkingScore(state));
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
    if (state.players.empty() || state.rankedPlayerIndices.empty()) return;

    int currentPlayerIdx = state.currentPlayerIndex;

    // Find the rank of the current player based on the rankedPlayerIndices list
    // Rank is 1-based index of their position in the sorted array
    int p1Rank = 1;
    for (size_t i = 0; i < state.rankedPlayerIndices.size(); ++i) {
        if (state.rankedPlayerIndices[i] == currentPlayerIdx) {
            p1Rank = i + 1;
            break;
        }
    }

    int compRankIdx = state.currentCompetitorRank;
    if (compRankIdx < 0 || compRankIdx >= (int)state.rankedPlayerIndices.size()) {
        compRankIdx = 0;
    }

    int competitorIdx = state.rankedPlayerIndices[compRankIdx];
    int p2Rank = compRankIdx + 1; // 1-based index

    char p1Place[16];
    char p2Place[16];
    getOrdinalString(p1Rank, p1Place, sizeof(p1Place));
    getOrdinalString(p2Rank, p2Place, sizeof(p2Place));

    displays.textDisplay.printHeadToHeadScreen(
        p1Place,
        &state.players[currentPlayerIdx].name,
        state.players[currentPlayerIdx].hue,
        p2Place,
        &state.players[competitorIdx].name,
        state.players[competitorIdx].hue
    );
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

void InGamePhase::endTurn(GameState& state) {
    state.currentPlayerIndex = (state.currentPlayerIndex + 1) % state.players.size();
}
