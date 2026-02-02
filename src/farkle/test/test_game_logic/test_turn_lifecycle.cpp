#include "test_turn_lifecycle.h"
#include "Game.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that a standard turn correctly banks the score and advances to the next player.
void test_TurnLifecycle_StandardTurn() {
    Game game;
    game.setup();
    game.state.atRiskScore = 500;
    game.controlPad.press(ButtonAction::BANK);

    for (int i = 0; i < 400; i++) {
        advance_millis(10);
        game.loop();
    }

    game.controlPad.press(ButtonAction::BANK);
    advance_millis(10);
    game.loop();

    TEST_ASSERT_EQUAL_INT(1, game.state.currentPlayerIndex);
    TEST_ASSERT_EQUAL_INT(500, game.state.players[0].score);
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
}

// Verifies that the game correctly cycles through all players.
void test_TurnLifecycle_RoundRobin() {
    Game game;
    game.setup();

    for (int i = 0; i < 4; i++) {
        game.state.atRiskScore = 500;
        game.controlPad.press(ButtonAction::BANK);

        for (int j = 0; j < 400; j++) {
            advance_millis(10);
            game.loop();
        }
    }

    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);
}

void run_turn_lifecycle_tests() {
    RUN_TEST(test_TurnLifecycle_StandardTurn);
    RUN_TEST(test_TurnLifecycle_RoundRobin);
}
