#include "phases/BankingPhase.h"
#include "Game.h"

// Constants for animation
const float SCORE_ANIMATION_SPEED = 0.5f; // points per millisecond (approx 500 points/sec)

void BankingPhase::onEnter(GameState& state) {
    scoreMoveAccumulator = 0.0f;
}

GamePhase* BankingPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    // 1. Perform Animation
    scoreMoveAccumulator += (SCORE_ANIMATION_SPEED * deltaTime);
    int pointsToMove = (int)scoreMoveAccumulator;
    
    if (pointsToMove > 0) {
        // Ensure we don't move more than what's left in atRiskScore
        if (pointsToMove > state.atRiskScore) {
            pointsToMove = state.atRiskScore;
        }
        
        state.players[state.currentPlayerIndex].score += pointsToMove;
        state.atRiskScore -= pointsToMove;
        scoreMoveAccumulator -= (float)pointsToMove;
    }

    // 2. Check for completion
    if (state.atRiskScore <= 0) {
        state.atRiskScore = 0; // Clean up any fractional remainder

        // Reset farkle count on a successful bank
        state.players[state.currentPlayerIndex].farkle_count = 0;

        // Check for Final Round Trigger
        if (!state.finalRoundTriggered && state.players[state.currentPlayerIndex].score >= 5000) {
            state.finalRoundTriggered = true;
        }

        // Advance turn and transition
        this->endTurn(state);
        return game.getPhase<WaitingPhase>();
    }

    return this;
}
