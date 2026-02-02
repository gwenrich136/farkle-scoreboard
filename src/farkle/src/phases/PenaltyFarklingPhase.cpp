#include "phases/PenaltyFarklingPhase.h"
#include "Game.h"

// Constants for animation
const float PENALTY_DRAIN_SPEED = 1.0f; // faster drain for farkles

void PenaltyFarklingPhase::onEnter(GameState& state) {
    scoreMoveAccumulator = 0.0f;

    // Apply the 1000-point penalty
    state.players[state.currentPlayerIndex].score -= 1000;
    if (state.players[state.currentPlayerIndex].score < 0) {
        state.players[state.currentPlayerIndex].score = 0;
    }

    // Reset the farkle count
    state.players[state.currentPlayerIndex].farkle_count = 0;
}

GamePhase* PenaltyFarklingPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    // 1. Perform Animation
    if (state.atRiskScore > 0) {
        scoreMoveAccumulator += (PENALTY_DRAIN_SPEED * deltaTime);
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
