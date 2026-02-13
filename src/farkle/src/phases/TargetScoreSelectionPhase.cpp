#include "phases/TargetScoreSelectionPhase.h"
#include "Game.h"
#include <cstdio>

void TargetScoreSelectionPhase::onEnter(GameState& state) {
    // Default is usually 10,000, which is set in GameState constructor or reset.
}

GamePhase* TargetScoreSelectionPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    if (action == ButtonAction::UP_1000) {
        state.targetScore += 1000;
    } else if (action == ButtonAction::DOWN_50) {
        state.targetScore -= 1000;
    } else if (action == ButtonAction::BANK || action == ButtonAction::FARKLE) {
        game.setTargetScore(state.targetScore);
        return game.getPhase<PlayerSelectionPhase>();
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
    char scoreStr[12];
    snprintf(scoreStr, sizeof(scoreStr), "%d", state.targetScore);
    displays.oled.printSelectionScreen("Target Score", scoreStr);
}
