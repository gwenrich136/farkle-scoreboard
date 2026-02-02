#include "test_full_game.h"
#include "Game.h"
#include "phases/PostGamePhase_V1.h"
#include <unity.h>
#include "Arduino.h"

// Simulates a full game where players take turns scoring until one player reaches the target score, triggering the final round.
void test_FullGame_StandardGame() {
    Game game;
    game.setup();

    int turn = 0;
    while(game.currentPhase != game.getPhase<PostGamePhase_V1>() && turn < 100) {
        game.controlPad.press(ButtonAction::UP_1000);
        advance_millis(10);
        game.loop();
        game.controlPad.press(ButtonAction::RIGHT_500);
        advance_millis(10);
        game.loop();
        game.controlPad.press(ButtonAction::BANK);

        while(game.state.atRiskScore > 0) {
            advance_millis(10);
            game.loop();
        }

        // Press a button to advance the turn
        game.controlPad.press(ButtonAction::BANK);
        advance_millis(10);
        game.loop();

        turn++;
    }

    TEST_ASSERT_LESS_THAN(100, turn); // Game should end in a reasonable number of turns
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PostGamePhase_V1>(), game.currentPhase);
}

void run_full_game_tests() {
    RUN_TEST(test_FullGame_StandardGame);
}
