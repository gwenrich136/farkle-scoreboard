#include "phases/EndOfTurnPhase.h"
#include "Game.h"
#include <assert.h>

void EndOfTurnPhase::onEnter(Game& game, GameState& state) {
    // Phase starts when animation is over.
    // Assert that the animation phase cleaned up the atRiskScore
    assert(state.atRiskScore == 0 && "atRiskScore must be 0 when entering EndOfTurnPhase");

    m_elapsedTime = 0;
}

GamePhase* EndOfTurnPhase::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    m_elapsedTime += deltaTime;

    // Wait for user dismissal (any button press, or scroll) or timeout
    if (input.action != ButtonAction::NONE || input.rotationDelta != 0 || m_elapsedTime >= 5000) {
        // Check for Final Round Trigger
        if (!state.finalRoundTriggered && state.players[state.currentPlayerIndex].score >= state.targetScore) {
            state.finalRoundTriggered = true;
            game.getSoundPlayer().play(SFX_FINAL_ROUND_BELL);
        }

        // Advance turn and transition
        uint32_t record = TurnRecord::pack(
            state.players[state.currentPlayerIndex].score,
            state.currentPlayerIndex,
            state.players[state.currentPlayerIndex].farkle_count,
            state.finalRoundTriggered,
            state.penaltyAppliedThisTurn
        );
        game.getMemoryCard().appendTurnRecord(record);
        
        // Clear flag for the next turn
        state.penaltyAppliedThisTurn = false;

        this->endTurn(state);
        return game.getPhase<WaitingPhase>();
    }

    return this;
}

void EndOfTurnPhase::updateAtRiskScoreDisplay(const GameState& state, const Displays& displays) {
    // Explicitly clear the at risk score display once the turn ends
    displays.scoreDisplay.clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
}

void EndOfTurnPhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    // Do nothing. Maintain the text from the previous phase.
}
