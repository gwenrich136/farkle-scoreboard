#include "phases/BankingPhase.h"
#include "Game.h"

// Constants for animation
const float SCORE_ANIMATION_SPEED = 0.5f; // points per millisecond (approx 500 points/sec)

void BankingPhase::onEnter(Game& game, GameState& state) {
    scoreMoveAccumulator = 0.0f;
    state.players[state.currentPlayerIndex].farkle_count = 0;
    game.getSoundPlayer().play(SoundEffect::SFX_BANKING);
}

GamePhase* BankingPhase::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    // 1. Perform Animation
    if (state.atRiskScore > 0) {
        scoreMoveAccumulator += (SCORE_ANIMATION_SPEED * deltaTime);
        int pointsToMove = (int)scoreMoveAccumulator;
        
        if (pointsToMove > 0) {
            // Ensure we don't move more than what's left in atRiskScore
            if (pointsToMove > state.atRiskScore) {
                pointsToMove = state.atRiskScore;
            }

            state.addPlayerScore(state.currentPlayerIndex, pointsToMove);
            state.atRiskScore -= pointsToMove;
            scoreMoveAccumulator -= (float)pointsToMove;
        }
    }

    // 2. Check for completion
    if (state.atRiskScore <= 0) {
        state.atRiskScore = 0; // Clean up any fractional remainder
        game.getSoundPlayer().stop();
        return game.getPhase<EndOfTurnPhase>();
    }

    return this;
}
