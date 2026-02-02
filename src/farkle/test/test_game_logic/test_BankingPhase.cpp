#include "test_BankingPhase.h"
#include "Game.h"
#include "phases/BankingPhase.h"
#include "phases/WaitingPhase.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that the score animation correctly moves points from atRiskScore to the player's score.
void test_BankingPhase_AnimationMath() {
    Game game;
    game.setup();
    game.state.atRiskScore = 500;
    game.state.players[0].score = 0;
    game.currentPhase = game.getPhase<BankingPhase>();

    for (int i = 0; i < 101; i++) { // 101 * 10ms = 1010ms, which is enough for 500 points (needs 1000ms)
        advance_millis(10);
        game.loop();
    }

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(500, game.state.players[0].score);
}

// Verifies that atRiskScore does not go negative when the points to be moved in one loop are greater than the remaining atRiskScore.
void test_BankingPhase_ZeroFloorSafety() {
    Game game;
    game.setup();
    game.state.atRiskScore = 50;
    game.state.players[0].score = 0;
    game.currentPhase = game.getPhase<BankingPhase>();

    advance_millis(1000); // 1000ms should be enough to drain all 50 points in one loop
    game.loop();

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(50, game.state.players[0].score);
}

// Verifies that button presses are ignored while the banking animation is in progress.
void test_BankingPhase_InputSpamming() {
    Game game;
    game.setup();
    game.state.atRiskScore = 500;
    game.currentPhase = game.getPhase<BankingPhase>();

    game.controlPad.press(ButtonAction::BANK);
    advance_millis(10);
    game.loop();
    game.controlPad.press(ButtonAction::CLEAR);
    advance_millis(10);
    game.loop();
    game.controlPad.press(ButtonAction::UP_1000);
    advance_millis(10);
    game.loop();

    TEST_ASSERT_EQUAL_PTR(game.getPhase<BankingPhase>(), game.currentPhase);
}

// Verifies that a button press transitions to the WaitingPhase after the banking animation is complete.
void test_BankingPhase_ManualAdvance() {
    Game game;
    game.setup();
    game.state.atRiskScore = 0;
    game.currentPhase = game.getPhase<BankingPhase>();

    advance_millis(10);
    game.loop();
    TEST_ASSERT_EQUAL_PTR(game.getPhase<BankingPhase>(), game.currentPhase);

    game.controlPad.press(ButtonAction::BANK);
    advance_millis(10);
    game.loop();

    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
}

void run_banking_phase_tests() {
    RUN_TEST(test_BankingPhase_AnimationMath);
    RUN_TEST(test_BankingPhase_ZeroFloorSafety);
    RUN_TEST(test_BankingPhase_InputSpamming);
    RUN_TEST(test_BankingPhase_ManualAdvance);
}
