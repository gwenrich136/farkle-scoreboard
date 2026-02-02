#include "test_full_game.h"
#include "Game.h"
#include "phases/WaitingPhase.h"
#include <unity.h>
#include "Arduino.h"

// Simulates a full game where players take turns scoring until one player reaches the target score, triggering the final round.
void test_FullGame_StandardGame() {
    Game game;
    game.setup();

    int turn = 0;
    while(game.currentPhase == game.getPhase<WaitingPhase>() && turn < 100) {
        game.state.atRiskScore = 1500;
        game.controlPad.press(ButtonAction::BANK);

        // Loop until the banking animation is complete and the turn has advanced
        for(int i = 0; i < 400; i++) {
            advance_millis(10);
            game.loop();
        }
        turn++;
    }

    TEST_ASSERT_LESS_THAN(100, turn); // Game should end in a reasonable number of turns
}

void run_full_game_tests() {
    RUN_TEST(test_FullGame_StandardGame);
}
