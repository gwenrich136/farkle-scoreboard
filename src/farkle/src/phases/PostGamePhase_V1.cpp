#include "phases/PostGamePhase_V1.h"

void PostGamePhase_V1::onEnter(GameState& state) {
    // Determine winner and cache display data
    m_winnerIdx = 0;
    m_highestScore = -1;
    for (int i = 0; i < state.players.size(); ++i) {
        if (state.players[i].score > m_highestScore) {
            m_highestScore = state.players[i].score;
            m_winnerIdx = i;
        }
    }

    m_winnerMsg = state.players[m_winnerIdx].name + " WINS!";

    // Update cached scores
    m_scores.clear();
    for (const auto& player : state.players) {
        m_scores.push_back(player.score);
    }
}

GamePhase* PostGamePhase_V1::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    // Frozen state: ignore all input
    return this;
}

void PostGamePhase_V1::display(const GameState& state, const Displays& displays) {
    // Display the winning player's name and freeze the grid/scores
    displays.oled.print(m_winnerMsg.c_str());

    // Update scores on the 7-segments one last time
    displays.scoreDisplay.print_number(0, 0); // At risk is 0
    displays.scoreDisplay.print_number(state.players[m_winnerIdx].score, 1); // Winner's score
    displays.scoreDisplay.print_number(m_highestScore, 2); // High score

    // Update the grid with final scores
    displays.grid.update(m_scores, m_winnerIdx, 0);
}
