#include "test_BankingPhase.h"
#include "Game.h"
#include "phases/BankingPhase.h"
#include "phases/WaitingPhase.h"
#include <unity.h>
#include "Arduino.h"

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

void test_BankingPhase_ZeroFloorSafety() {
    Game game;
    game.setup();
    game.state.atRiskScore = 50;
    game.state.players[0].score = 0;
    game.currentPhase = game.getPhase<BankingPhase>();

    for (int i = 0; i < 11; i++) { // 11 * 10ms = 110ms, which is enough for 50 points (needs 100ms)
        advance_millis(10);
        game.loop();
    }

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(50, game.state.players[0].score);
}

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
