#include "phases/StartupPhase.h"
#include "Game.h"

void StartupPhase::onEnter(GameState& state) {
    // Legacy support, use Game overload
}

void StartupPhase::onEnter(Game& game, GameState& state) {
    // PreGamePhase doesn't override onEnter, so no base call needed.
    _hasActiveGame = game.getMemoryCard().hasActiveGame();
    _selectionIndex = 0;
}

GamePhase* StartupPhase::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    if (_hasActiveGame) {
        if (input.rotationDelta > 0) {
            _selectionIndex = (_selectionIndex + 1) % 2;
        } else if (input.rotationDelta < 0) {
            _selectionIndex = (_selectionIndex - 1 + 2) % 2;
        }
    } else {
        _selectionIndex = 0; // Only "New Game"
    }

    if (input.action == ButtonAction::SELECT) {
        if (_hasActiveGame && _selectionIndex == 0) {
            // Resume Game
            if (game.getMemoryCard().loadGameMetadata(state) &&
                game.getMemoryCard().replayGameJournal(state)) {
                return game.getPhase<WaitingPhase>();
            } else {
                // If it failed to resume, fall back to new game
                game.getMemoryCard().clearActiveGame();
                return game.getPhase<TargetScoreSelectionPhase>();
            }
        } else {
            // New Game
            game.getMemoryCard().clearActiveGame();
            return game.getPhase<TargetScoreSelectionPhase>();
        }
    }

    return this;
}

void StartupPhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    if (_hasActiveGame) {
        if (_selectionIndex == 0) {
            displays.textDisplay.printSelectionScreen("Farkle!", "Resume Game");
        } else {
            displays.textDisplay.printSelectionScreen("Farkle!", "New Game");
        }
    } else {
        displays.textDisplay.printSelectionScreen("Farkle!", "New Game");
    }
}
