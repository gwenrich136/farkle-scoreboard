#include "phases/StartupPhase.h"
#include "Game.h"

void StartupPhase::onEnter(Game& game, GameState& state) {
    populateOptions(game);
    _selectionIndex = 0;
}

void StartupPhase::populateOptions(Game& game) {
    _options.clear();
    if (game.getMemoryCard().hasActiveGame()) {
        _options.push_back(StartupOption::RESUME_GAME);
    }
    _options.push_back(StartupOption::NEW_GAME);
}

StartupOption StartupPhase::getSelectedOption() const {
    if (_options.empty()) return StartupOption::NEW_GAME;
    return _options[_selectionIndex];
}

GamePhase* StartupPhase::launchResumeGame(Game& game, GameState& state) {
    if (game.getMemoryCard().loadGameMetadata(state) &&
        game.getMemoryCard().replayGameJournal(state)) {
        game.getSoundPlayer().play(SFX_RESUME_GAME);
        game.resumeGameDisplays();
        return game.getPhase<WaitingPhase>();
    } else {
        // If it failed to resume, fall back to StartupPhase to let user pick again
        game.getMemoryCard().clearActiveGame();
        populateOptions(game);
        _selectionIndex = 0;
        return this;
    }
}

GamePhase* StartupPhase::launchStartNewGame(Game& game, GameState& state) {
    game.getSoundPlayer().play(SFX_NEW_GAME);
    // Only clears curr_id file, not the partial game files which happens on player selection finalize
    return game.getPhase<TargetScoreSelectionPhase>();
}

GamePhase* StartupPhase::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    if (_options.size() > 1) {
        if (input.rotationDelta > 0) {
            _selectionIndex = (_selectionIndex + 1) % _options.size();
        } else if (input.rotationDelta < 0) {
            _selectionIndex = (_selectionIndex - 1 + _options.size()) % _options.size();
        }
    } else {
        _selectionIndex = 0;
    }

    if (input.action == ButtonAction::SELECT) {
        switch (getSelectedOption()) {
            case StartupOption::RESUME_GAME:
                return launchResumeGame(game, state);
            case StartupOption::NEW_GAME:
            default:
                return launchStartNewGame(game, state);
        }
    }

    return this;
}

void StartupPhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    switch (getSelectedOption()) {
        case StartupOption::RESUME_GAME:
            displays.textDisplay.printSelectionScreen("Farkle!", "Resume Game");
            break;
        case StartupOption::NEW_GAME:
        default:
            displays.textDisplay.printSelectionScreen("Farkle!", "New Game");
            break;
    }
}
