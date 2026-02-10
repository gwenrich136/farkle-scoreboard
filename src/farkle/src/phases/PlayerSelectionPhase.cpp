#include "phases/PlayerSelectionPhase.h"
#include "Game.h"
#include <algorithm>

const std::vector<std::string> PlayerSelectionPhase::s_namePool = {
    "Geewee", "Sammy", "Coach", "Sheshe", "Alex", "Tigre", "Pepa", "Fred", "Andrea"
};

void PlayerSelectionPhase::onEnter(GameState& state) {
    m_selectionIndex = 0;
}

GamePhase* PlayerSelectionPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    std::vector<std::string> availableNames = getAvailableNames(state);

    if (availableNames.empty()) {
        if (action == ButtonAction::FARKLE && state.players.size() >= 1) {
            return game.getPhase<WaitingPhase>();
        }
        return this;
    }

    // Wrap selection index
    if (action == ButtonAction::UP_1000) {
        m_selectionIndex = (m_selectionIndex + 1) % availableNames.size();
    } else if (action == ButtonAction::DOWN_50) {
        m_selectionIndex = (m_selectionIndex - 1 + availableNames.size()) % availableNames.size();
    } else if (action == ButtonAction::BANK) {
        if (game.canAddPlayer()) {
            game.addPlayer(availableNames[m_selectionIndex]);
            m_selectionIndex = 0; // Reset selection index after adding
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
    std::vector<std::string> availableNames = getAvailableNames(state);
    if (displays.grid.isMaxPlayersReached()) {
        displays.oled.printSelectionScreen("Add Player", "ROSTER FULL");
    } else if (availableNames.empty()) {
        displays.oled.printSelectionScreen("Add Player", "NO MORE NAMES");
    } else {
        displays.oled.printSelectionScreen("Add Player", availableNames[m_selectionIndex].c_str());
    }
}

std::vector<std::string> PlayerSelectionPhase::getAvailableNames(const GameState& state) const {
    std::vector<std::string> available;
    for (const auto& name : s_namePool) {
        bool alreadyAdded = false;
        for (const auto& player : state.players) {
            if (player.name == name) {
                alreadyAdded = true;
                break;
            }
        }
        if (!alreadyAdded) {
            available.push_back(name);
        }
    }
    return available;
}
