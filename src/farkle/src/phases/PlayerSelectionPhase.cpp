#include "phases/PlayerSelectionPhase.h"
#include "Game.h"
#include <algorithm>

const std::vector<std::string> PlayerSelectionPhase::s_namePool = {
    "Geewee", "Sammy", "Coach", "Sheshe", "Alex", "Tigre", "Pepa", "Fred", "Andrea"
};

void PlayerSelectionPhase::onEnter(GameState& state) {
    m_selectionIndex = 0;
    updateAvailableNames(state);
}

GamePhase* PlayerSelectionPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    updateAvailableNames(state);

    if (m_availableNames.empty()) {
        if (action == ButtonAction::FARKLE && state.players.size() >= 1) {
            return game.getPhase<WaitingPhase>();
        }
        return this;
    }

    // Wrap selection index
    if (action == ButtonAction::UP_1000) {
        m_selectionIndex = (m_selectionIndex + 1) % m_availableNames.size();
    } else if (action == ButtonAction::DOWN_50) {
        m_selectionIndex = (m_selectionIndex - 1 + m_availableNames.size()) % m_availableNames.size();
    } else if (action == ButtonAction::BANK) {
        if (game.canAddPlayer()) {
            game.addPlayer(m_availableNames[m_selectionIndex]);
            updateAvailableNames(state); // Update immediately to reflect changes

            // Adjust index if it's now out of bounds due to the smaller list
            if (!m_availableNames.empty() && m_selectionIndex >= (int)m_availableNames.size()) {
                m_selectionIndex = 0;
            }
        }
    } else if (action == ButtonAction::FARKLE) {
        if (state.players.size() >= 1) {
            return game.getPhase<WaitingPhase>();
        }
    }

    return this;
}

void PlayerSelectionPhase::updateProgressGrid(const GameState& state, const Displays& displays) {
    displays.grid.displayPlayersPregame(!displays.grid.isMaxPlayersReached());
}

void PlayerSelectionPhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    // In the current configuration (pool=9, max=8), the list will never be empty before the roster is full.
    // However, we merge the conditions here as requested to simplify the logic.
    if (displays.grid.isMaxPlayersReached() || m_availableNames.empty()) {
        displays.oled.print("ROSTER FULL");
    } else {
        displays.oled.printSelectionScreen("Add Player", m_availableNames[m_selectionIndex].c_str());
    }
}

void PlayerSelectionPhase::updateAvailableNames(const GameState& state) {
    m_availableNames.clear();
    for (const auto& name : s_namePool) {
        bool alreadyAdded = false;
        for (const auto& player : state.players) {
            if (player.name == name) {
                alreadyAdded = true;
                break;
            }
        }
        if (!alreadyAdded) {
            m_availableNames.push_back(name);
        }
    }
}
