#include "phases/PlayerSelectionPhase.h"
#include "Game.h"
#include <algorithm>
#include "GameConstants.h"
#include <string.h>

void PlayerSelectionPhase::onEnter(GameState& state) {
    m_currentSelection = nullptr;
}

GamePhase* PlayerSelectionPhase::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    // Lazy initialization on first frame
    if (m_currentSelection == nullptr) {
        game.getMemoryCard().beginPlayerSelection();
        m_currentSelection = game.getMemoryCard().getCurrentPlayer();
    }

    // Handle rotation for selection
    if (input.action == ButtonAction::NONE && input.rotationDelta != 0) {
        if (input.rotationDelta > 0) {
            for (int i = 0; i < input.rotationDelta; ++i) {
                m_currentSelection = game.getMemoryCard().getNextPlayer();
            }
        } else if (input.rotationDelta < 0) {
            for (int i = 0; i > input.rotationDelta; --i) {
                m_currentSelection = game.getMemoryCard().getPreviousPlayer();
            }
        }
    }

    // Handle actions
    if (input.action == ButtonAction::SELECT) {
        if (game.canAddPlayer()) {
            if (m_currentSelection != nullptr && m_currentSelection[0] != '\0') {
                char reservedName[13];
                game.getMemoryCard().reservePlayer(reservedName);
                game.addPlayer(reservedName);

                // Update current selection to whatever MemoryCard auto-advanced to
                m_currentSelection = game.getMemoryCard().getCurrentPlayer();
            }
        }
    } else if (input.action == ButtonAction::FARKLE) {
        if (state.players.size() >= 1) {
            game.getMemoryCard().finalizeSelection();
            return game.getPhase<WaitingPhase>();
        }
    }

    return this;
}

void PlayerSelectionPhase::updateProgressGrid(const GameState& state, const Displays& displays) {
    bool isRosterFull = state.players.size() >= MAX_PLAYERS;
    if (!isRosterFull) {
        displays.grid.displayPlayersPregame(state.getNextPlayerHue(state.players.size()));
    } else {
        displays.grid.displayPlayersPregame(std::nullopt);
    }
}

void PlayerSelectionPhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    bool isRosterFull = state.players.size() >= MAX_PLAYERS;
    if (isRosterFull) {
        displays.textDisplay.print("ROSTER FULL");
    } else if (m_currentSelection == nullptr || m_currentSelection[0] == '\0') {
        displays.textDisplay.printSelectionScreen(
            "Add Player",
            "NEW PLAYER"
        );
    } else {
        displays.textDisplay.printSelectionScreen(
            "Add Player",
            m_currentSelection,
            state.getNextPlayerHue(state.players.size())
        );
    }
}
