#include "phases/TargetScoreSelectionPhase.h"
#include "Game.h"
#include <cstdio>

void TargetScoreSelectionPhase::onEnter(GameState& state) {
    // Default is usually 10,000, which is set in GameState constructor or reset.
}

GamePhase* TargetScoreSelectionPhase::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    if (input.action == ButtonAction::SELECT) {
        game.setTargetScore(state.targetScore);
        return game.getPhase<PlayerSelectionPhase>();
    }

    // Input Priority: Only process rotation if no digital action occurred
    // (Although ControlPad already suppresses rotation if action != NONE, explicit check is safer/cleaner)
    if (input.action == ButtonAction::NONE && input.rotationDelta != 0) {
        state.targetScore += input.rotationDelta * 1000;
    }

    // Clamp between 1,000 and 20,000
    if (state.targetScore < 1000) state.targetScore = 1000;
    if (state.targetScore > 20000) state.targetScore = 20000;

    return this;
}

void TargetScoreSelectionPhase::updateProgressGrid(const GameState& state, const Displays& displays) {
    // The grid is cleared by Game::resetGame() before entering this phase.
    // We don't need to do anything here to keep it clear.
}

void TargetScoreSelectionPhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    char scoreStr[16];
    // Format with thousand separator (clamped 1,000 to 20,000)
    snprintf(scoreStr, sizeof(scoreStr), "%d,%03d", state.targetScore / 1000, state.targetScore % 1000);
    displays.textDisplay.printSelectionScreen("Target Score", scoreStr);
}
