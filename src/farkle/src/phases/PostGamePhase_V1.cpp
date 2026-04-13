#include "phases/PostGamePhase_V1.h"
#include "Game.h"

void PostGamePhase_V1::onEnter(GameState& state) {
    // Winner is already determined by the sorted rankedPlayerIndices list from the start of the turn.
    m_winnerIdx = state.rankedPlayerIndices[0];
    m_highestScore = state.players[m_winnerIdx].score;

    m_winnerMsg = state.players[m_winnerIdx].name + " WINS!";

    // Update cached scores
    m_scoresCount = 0;
    for (const auto& player : state.players) {
        if (m_scoresCount < MAX_PLAYERS) {
            m_scores[m_scoresCount++] = player.score;
        }
    }
}

GamePhase* PostGamePhase_V1::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    // Frozen state: ignore all input
    return this;
}

void PostGamePhase_V1::display(const GameState& state, const Displays& displays) {
    // Display the winning player's name and freeze the grid/scores
    displays.textDisplay.print(m_winnerMsg.c_str(), state.players[m_winnerIdx].hue);

    // Update scores on the 7-segments one last time
    displays.scoreDisplay.clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
    displays.scoreDisplay.print_number(state.players[m_winnerIdx].score, ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE); // Winner's score
    displays.scoreDisplay.print_number(m_highestScore, ScoreDisplay::DisplayType::COMPETITION_SCORE, true); // High score (flashes for celebration)

    // Update the grid with final scores
    displays.grid.update(m_scores, m_scoresCount, m_winnerIdx, 0);
}
