#include "phases/FarklingPhase.h"
#include "Game.h"

// Constants for animation
const float FARKLE_DRAIN_SPEED = 1.0f; // faster drain for farkles

void FarklingPhase::onEnter(GameState& state) {
    scoreMoveAccumulator = 0.0f;
    
    // Increment farkle count (3 farkles in a row logic can be added here in future)
    state.players[state.currentPlayerIndex].farkle_count++;
    if (state.players[state.currentPlayerIndex].farkle_count > 2) {
        // Special penalty logic for 3rd farkle could go here
        state.players[state.currentPlayerIndex].farkle_count = 0;
    }
}

GamePhase* FarklingPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    // 1. Perform Animation
    scoreMoveAccumulator += (FARKLE_DRAIN_SPEED * deltaTime);
    int pointsToDrain = (int)scoreMoveAccumulator;

    if (pointsToDrain > 0) {
        if (pointsToDrain > state.atRiskScore) {
            pointsToDrain = state.atRiskScore;
        }
        state.atRiskScore -= pointsToDrain;
        scoreMoveAccumulator -= (float)pointsToDrain;
    }

    // 2. Check for completion
    if (state.atRiskScore <= 0) {
        state.atRiskScore = 0;
        this->endTurn(state);
        return game.getPhase<WaitingPhase>();
    }

    return this;
}
