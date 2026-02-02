#include "test_full_game.h"
#include "Game.h"
#include <unity.h>

void test_FullGame_StandardGame() {
    Game game;
    game.setup();

    // P1 scores
    game.state.players[0].score = 500;
    game.state.currentPlayerIndex = 0;
    game.loop();

    // P2's turn
    game.state.currentPlayerIndex = 1;
    game.loop();

    // P3's turn
    game.state.currentPlayerIndex = 2;
    game.loop();

    // P4's turn
    game.state.currentPlayerIndex = 3;
    game.loop();
}

void run_full_game_tests() {
    RUN_TEST(test_FullGame_StandardGame);
}
