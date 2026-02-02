#include "test_FarklingPhase.h"
#include "Game.h"
#include "phases/FarklingPhase.h"
#include "phases/WaitingPhase.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that the atRiskScore drains to 0 but does NOT add to player score.
void test_FarklingPhase_AnimationMath() {
    Game game;
    game.setup();
    game.state.atRiskScore = 500;
    game.state.players[0].score = 1000;
    game.currentPhase = game.getPhase<FarklingPhase>();
    game.currentPhase->onEnter(game.state);

    for (int i = 0; i < 101; i++) { // 101 * 10ms = 1010ms
        simulateNoAction(game);
    }

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(1000, game.state.players[0].score); // Score should NOT change
}

// Verifies that atRiskScore does not go negative.
void test_FarklingPhase_ZeroFloorSafety() {
    Game game;
    game.setup();
    game.state.atRiskScore = 50;
    game.currentPhase = game.getPhase<FarklingPhase>();
    game.currentPhase->onEnter(game.state);

    simulateNoAction(game, 1000);

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
}

// Verifies that button presses are ignored while the farkling animation is in progress.
void test_FarklingPhase_InputSpamming() {
    Game game;
    game.setup();
    game.state.atRiskScore = 500;
    game.currentPhase = game.getPhase<FarklingPhase>();
    game.currentPhase->onEnter(game.state);

    simulateButtonPress(game, ButtonAction::BANK);
    simulateButtonPress(game, ButtonAction::CLEAR);
    simulateButtonPress(game, ButtonAction::UP_1000);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<FarklingPhase>(), game.currentPhase);
}

// Verifies that a button press transitions to the WaitingPhase after the animation is complete.
void test_FarklingPhase_ManualAdvance() {
    Game game;
    game.setup();
    game.state.atRiskScore = 0;
    game.currentPhase = game.getPhase<FarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.currentPlayerIndex = 0;

    simulateNoAction(game);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<FarklingPhase>(), game.currentPhase);

    simulateButtonPress(game, ButtonAction::CLEAR);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
}

void run_farkling_phase_tests() {
    RUN_TEST(test_FarklingPhase_AnimationMath);
    RUN_TEST(test_FarklingPhase_ZeroFloorSafety);
    RUN_TEST(test_FarklingPhase_InputSpamming);
    RUN_TEST(test_FarklingPhase_ManualAdvance);
}