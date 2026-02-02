#include "phases/FarklingPhase.h"
#include "Game.h"

// Constants for animation
const float FARKLE_DRAIN_SPEED = 1.0f; // faster drain for farkles

void FarklingPhase::onEnter(Game& game, GameState& state) {
    scoreMoveAccumulator = 0.0f;
    
    state.players[state.currentPlayerIndex].farkle_count++;
    if (state.players[state.currentPlayerIndex].farkle_count > 2) {
        state.catastrophicFarkle = true;
        // In a catastrophic farkle, the player loses all their points
        state.players[state.currentPlayerIndex].score = 0;
        state.players[state.currentPlayerIndex].farkle_count = 0;
    }
}

GamePhase* FarklingPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    // 1. Handle catastrophic farkle visuals
    if (state.catastrophicFarkle) {
        game.farkleLights.alternate();
    }

    // 2. Perform Animation
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

    // 3. Check for completion
    if (state.atRiskScore <= 0) {
        state.atRiskScore = 0;

        // Wait for user dismissal
        if (action != ButtonAction::NONE) {
            state.catastrophicFarkle = false; // Reset the flag
            this->endTurn(state);
            return game.getPhase<WaitingPhase>();
        }
    }

    return this;
}
