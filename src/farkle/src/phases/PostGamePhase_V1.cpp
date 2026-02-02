#include "phases/PostGamePhase_V1.h"
#include <vector>

void PostGamePhase_V1::onEnter(GameState& state) {
    // No specific local state
}

GamePhase* PostGamePhase_V1::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    // Frozen state: ignore all input
    return this;
}

void PostGamePhase_V1::display(const GameState& state, const Displays& displays) {
    // Display the winning player's name and freeze the grid/scores
    int winnerIdx = 0;
    int highestScore = -1;
    for (int i = 0; i < state.players.size(); ++i) {
        if (state.players[i].score > highestScore) {
            highestScore = state.players[i].score;
            winnerIdx = i;
        }
    }

    std::string winnerMsg = state.players[winnerIdx].name + " WINS!";
    displays.oled.print(winnerMsg.c_str());

    // Update scores on the 7-segments one last time
    displays.scoreDisplay.print_number(0, 0); // At risk is 0
    displays.scoreDisplay.print_number(state.players[winnerIdx].score, 1); // Winner's score
    displays.scoreDisplay.print_number(highestScore, 2); // High score

    // Update the grid with final scores
    std::vector<int> scores;
    for (const auto& player : state.players) {
        scores.push_back(player.score);
    }
    displays.grid.update(scores, winnerIdx, 0);
}
