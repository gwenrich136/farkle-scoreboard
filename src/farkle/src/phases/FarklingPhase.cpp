#include "phases/FarklingPhase.h"
#include "Game.h"

// Constants for animation
const float FARKLE_DRAIN_SPEED = 1.0f;

void FarklingPhase::onEnter(GameState& state) {
    scoreMoveAccumulator = 0.0f;
    // No Harm, No Foul: only increment farkle_count if the player has points to lose.
    if (state.players[state.currentPlayerIndex].score > 0) {
        state.players[state.currentPlayerIndex].farkle_count++;
    }
}

GamePhase* FarklingPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    // 1. Perform Animation
    if (state.atRiskScore > 0) {
        scoreMoveAccumulator += (FARKLE_DRAIN_SPEED * deltaTime);
        int pointsToDrain = (int)scoreMoveAccumulator;

        if (pointsToDrain > 0) {
            if (pointsToDrain > state.atRiskScore) {
                pointsToDrain = state.atRiskScore;
            }
            state.atRiskScore -= pointsToDrain;
            scoreMoveAccumulator -= (float)pointsToDrain;
        }
    }

    // 2. Check for completion
    if (state.atRiskScore <= 0) {
        state.atRiskScore = 0;

        // Wait for user dismissal
        if (action != ButtonAction::NONE) {
            this->endTurn(state);
            return game.getPhase<WaitingPhase>();
        }
    }

    return this;
}

void FarklingPhase::updateAtRiskScoreDisplay(const GameState& state, const Displays& displays) {
    displays.scoreDisplay.clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
}

void FarklingPhase::updateCurrentPlayerScoreDisplay(const GameState& state, const Displays& displays) {
    displays.scoreDisplay.print_number(state.players[state.currentPlayerIndex].score + state.atRiskScore, ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE);
}

void FarklingPhase::updateProgressGrid(const GameState& state, const Displays& displays, bool includeAtRisk, bool blinkAtRisk) {
    // Farkling phase shows the potential score (no blinking)
    InGamePhase::updateProgressGrid(state, displays, true, false);
}
