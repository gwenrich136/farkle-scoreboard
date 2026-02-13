#include "phases/PostGamePhase_V1.h"

void PostGamePhase_V1::onEnter(GameState& state) {
    // Determine winner and cache display data
    // Tie-breaking: first person to reach the high score in the rotation wins.
    // We start searching from the current player, who is the one who reached the target first.
    m_highestScore = state.players[state.currentPlayerIndex].score;
    m_winnerIdx = state.currentPlayerIndex;
    int numPlayers = (int)state.players.size();

    for (int i = (state.currentPlayerIndex + 1) % numPlayers; i != state.currentPlayerIndex; i = (i + 1) % numPlayers) {
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
    displays.scoreDisplay.clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
    displays.scoreDisplay.print_number(state.players[m_winnerIdx].score, ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE); // Winner's score
    displays.scoreDisplay.print_number(m_highestScore, ScoreDisplay::DisplayType::COMPETITION_SCORE, true); // High score (flashes for celebration)

    // Update the grid with final scores
    displays.grid.update(m_scores, m_winnerIdx, 0);
}
