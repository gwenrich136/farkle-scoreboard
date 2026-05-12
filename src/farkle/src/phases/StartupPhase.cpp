#include "phases/StartupPhase.h"
#include "Game.h"

void StartupPhase::onEnter(GameState& state) {
    // Initial dummy state
}

GamePhase* StartupPhase::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    if (input.action == ButtonAction::SELECT) {
        return game.getPhase<TargetScoreSelectionPhase>();
    }

    return this;
}

void StartupPhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    displays.textDisplay.printSelectionScreen("Farkle!", "New Game");
}
