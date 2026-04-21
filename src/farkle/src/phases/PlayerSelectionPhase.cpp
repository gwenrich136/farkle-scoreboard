#include "phases/PlayerSelectionPhase.h"
#include "Game.h"
#include <algorithm>
#include "GameConstants.h"
#include <string.h>

void PlayerSelectionPhase::onEnter(GameState& state) {
    m_currentSelection[0] = '\0';
}

GamePhase* PlayerSelectionPhase::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    // Lazy initialization on first frame
    if (m_currentSelection[0] == '\0') {
        game.memoryCard.beginPlayerSelection();
        const char* firstPlayer = game.memoryCard.getCurrentPlayer();
        if (firstPlayer && firstPlayer[0] != '\0') {
            strncpy(m_currentSelection, firstPlayer, 12);
            m_currentSelection[12] = '\0';
        } else {
            // Fallback if list is empty but we just entered
            strcpy(m_currentSelection, "NO PLAYERS");
        }

        // Return this frame so the display logic has a chance to run with the new selection
        return this;
    }

    if (m_currentSelection[0] == '\0' || strcmp(m_currentSelection, "NO PLAYERS") == 0) {
        if (input.action == ButtonAction::FARKLE && state.players.size() >= 1) {
            game.memoryCard.finalizeSelection();
            return game.getPhase<WaitingPhase>();
        }
        return this;
    }

    // Handle rotation for selection
    if (input.action == ButtonAction::NONE && input.rotationDelta != 0) {
        const char* nextName = m_currentSelection;
        if (input.rotationDelta > 0) {
            for (int i = 0; i < input.rotationDelta; ++i) {
                const char* n = game.memoryCard.getNextPlayer();
                if (!n || n[0] == '\0') {
                    // Note: previously we called beginPlayerSelection() to wrap, but that resets reservations!
                    // MemoryCard cursor management handles bounds already (returns empty string). To wrap safely:
                    // we must iterate from the start of the list using getPreviousPlayer until it hits start (-1).
                    while (game.memoryCard.getPreviousPlayer()[0] != '\0') {}
                    n = game.memoryCard.getNextPlayer(); // Gets first valid
                }
                if (n && n[0] != '\0') nextName = n;
            }
        } else if (input.rotationDelta < 0) {
            for (int i = 0; i > input.rotationDelta; --i) {
                const char* p = game.memoryCard.getPreviousPlayer();
                if (!p || p[0] == '\0') {
                    // Wrap to end without destroying reservations
                    while (game.memoryCard.getNextPlayer()[0] != '\0') {}
                    p = game.memoryCard.getPreviousPlayer(); // Gets last valid
                }
                if (p && p[0] != '\0') nextName = p;
            }
        }

        if (nextName) {
            strncpy(m_currentSelection, nextName, 12);
            m_currentSelection[12] = '\0';
        }
    }

    // Handle actions
    if (input.action == ButtonAction::SELECT) {
        if (game.canAddPlayer()) {
            if (m_currentSelection[0] != '\0') {
                char reservedName[13];
                game.memoryCard.reservePlayer(reservedName);
                game.addPlayer(reservedName);

                // Update current selection to whatever MemoryCard auto-advanced to
                const char* newSelection = game.memoryCard.getCurrentPlayer();
                if (!newSelection || newSelection[0] == '\0') {
                    // Wrapped around, start from beginning without resetting reservations
                    while (game.memoryCard.getPreviousPlayer()[0] != '\0') {}
                    newSelection = game.memoryCard.getNextPlayer();
                }

                if (newSelection && newSelection[0] != '\0') {
                    strncpy(m_currentSelection, newSelection, 12);
                    m_currentSelection[12] = '\0';
                } else {
                    m_currentSelection[0] = '\0';
                }
            }
        }
    } else if (input.action == ButtonAction::FARKLE) {
        if (state.players.size() >= 1) {
            game.memoryCard.finalizeSelection();
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
    if (isRosterFull || m_currentSelection[0] == '\0' || strcmp(m_currentSelection, "NO PLAYERS") == 0) {
        displays.textDisplay.print("ROSTER FULL");
    } else {
        displays.textDisplay.printSelectionScreen(
            "Add Player",
            m_currentSelection,
            state.getNextPlayerHue(state.players.size())
        );
    }
}
