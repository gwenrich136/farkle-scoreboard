#include "phases/PenaltyFarklingPhase.h"
#include "Game.h"

// Constants for animation
const float PENALTY_DRAIN_SPEED = 1.0f; // faster drain for farkles

void PenaltyFarklingPhase::onEnter(GameState& state) {
    scoreMoveAccumulator = 0.0f;
    state.atRiskScore = -1000;

    // Reset the farkle count
    state.players[state.currentPlayerIndex].farkle_count = 0;
}

GamePhase* PenaltyFarklingPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    // 1. Perform Animation
    if (state.atRiskScore < 0) {
        scoreMoveAccumulator += (PENALTY_DRAIN_SPEED * deltaTime);
        int pointsToAdd = (int)scoreMoveAccumulator;

        if (pointsToAdd > 0) {
            Player& currentPlayer = state.players[state.currentPlayerIndex];

            // Ensure we don't add more than what's left in atRiskScore (a negative value)
            if (pointsToAdd > -state.atRiskScore) {
                pointsToAdd = -state.atRiskScore;
            }

            // Ensure player score doesn't go below zero
            if (pointsToAdd > currentPlayer.score) {
                pointsToAdd = currentPlayer.score;
            }

            currentPlayer.score -= pointsToAdd;
            state.atRiskScore += pointsToAdd;
            scoreMoveAccumulator -= (float)pointsToAdd;
        }
    }

    // 2. Check for completion
    if (state.atRiskScore >= 0) {
        state.atRiskScore = 0; // Clean up any fractional remainder

        // Wait for user dismissal
        if (action != ButtonAction::NONE) {
            this->endTurn(state);
            return game.getPhase<WaitingPhase>();
        }
    }

    return this;
}
