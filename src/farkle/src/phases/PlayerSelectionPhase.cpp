#include "phases/PlayerSelectionPhase.h"
#include "Game.h"
#include "phases/WaitingPhase.h"
#include <algorithm>

PlayerSelectionPhase::PlayerSelectionPhase() : m_selectionIndex(0) {
    m_namePool = {"Geewee", "Sammy", "Coach", "Sheshe", "Alex", "Tigre", "Pepa", "Fred", "Andrea"};
}

void PlayerSelectionPhase::onEnter(GameState& state) {
    m_selectionIndex = 0;
}

std::vector<std::string> PlayerSelectionPhase::getFilteredNames(const GameState& state) {
    std::vector<std::string> filtered;
    for (const auto& name : m_namePool) {
        bool alreadyAdded = false;
        for (const auto& player : state.players) {
            if (player.name == name) {
                alreadyAdded = true;
                break;
            }
        }
        if (!alreadyAdded) {
            filtered.push_back(name);
        }
    }
    return filtered;
}

GamePhase* PlayerSelectionPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    auto filteredNames = getFilteredNames(state);

    if (action == ButtonAction::UP_1000) {
        if (!filteredNames.empty()) {
            m_selectionIndex = (m_selectionIndex + 1) % filteredNames.size();
        }
    } else if (action == ButtonAction::DOWN_50) {
        if (!filteredNames.empty()) {
            m_selectionIndex = (m_selectionIndex - 1 + filteredNames.size()) % filteredNames.size();
        }
    } else if (action == ButtonAction::BANK) {
        if (!filteredNames.empty() && game.canAddPlayer()) {
            game.addPlayer(filteredNames[m_selectionIndex]);
            m_selectionIndex = 0; // Reset selection index after adding
        }
    } else if (action == ButtonAction::FARKLE) {
        if (state.players.size() >= 1) {
            return game.getPhase<WaitingPhase>();
        }
    }

    return this;
}

void PlayerSelectionPhase::display(const GameState& state, const Displays& displays) {
    auto filteredNames = getFilteredNames(state);
    const char* currentSelection = filteredNames.empty() ? "MAX PLAYERS" : filteredNames[m_selectionIndex].c_str();

    displays.oled.printSelectionScreen("Add Player", currentSelection);
    displays.grid.displayPlayersPregame(!displays.grid.isMaxPlayersReached());

    displays.scoreDisplay.clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
    displays.scoreDisplay.clear(ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE);
    displays.scoreDisplay.clear(ScoreDisplay::DisplayType::COMPETITION_SCORE);
    displays.farkleLights.farkle_state(0);
}
