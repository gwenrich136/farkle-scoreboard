#include "test_PenaltyFarklingPhase.h"
#include "Game.h"
#include "phases/PenaltyFarklingPhase.h"
#include "phases/WaitingPhase.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that the score animation correctly moves points from atRiskScore (negative) to 0
// and subtracts from the player's score.
void test_PenaltyFarklingPhase_AnimationMath() {
    Game game;
    game.setup();
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.atRiskScore = -1000;
    game.state.players[0].score = 2000;

    // Needs enough time for flashing (if any) and animation.
    // Based on full_game test, 2000ms seems sufficient for 1000 points.
    for (int i = 0; i < 300; i++) { // 300 * 10ms = 3000ms
        simulateNoAction(game);
    }

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(1000, game.state.players[0].score);
}

// Verifies that atRiskScore does not go above 0 (Zero Ceiling).
void test_PenaltyFarklingPhase_ZeroCeilingSafety() {
    Game game;
    game.setup();
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.atRiskScore = -50;
    game.state.players[0].score = 2000;

    // Advance enough time to drain -50.
    // Be generous to cover potential flashing delays.
    for (int i = 0; i < 300; i++) {
        simulateNoAction(game);
    }

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(1950, game.state.players[0].score);
}

// Verifies that button presses are ignored while the animation is in progress.
void test_PenaltyFarklingPhase_InputSpamming() {
    Game game;
    game.setup();
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.atRiskScore = -500;

    simulateButtonPress(game, ButtonAction::BANK);
    simulateButtonPress(game, ButtonAction::CLEAR);
    simulateButtonPress(game, ButtonAction::UP_1000);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);
}

// Verifies that a button press transitions to the WaitingPhase after the animation is complete.
void test_PenaltyFarklingPhase_ManualAdvance() {
    Game game;
    game.setup();
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.atRiskScore = 0; // Animation done
    
    // Ensure we are past any internal timers if necessary.
    // If atRiskScore is manually set to 0, logic should consider it done.

    simulateNoAction(game);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);

    simulateButtonPress(game, ButtonAction::CLEAR);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
}

void run_penalty_farkling_phase_tests() {
    RUN_TEST(test_PenaltyFarklingPhase_AnimationMath);
    RUN_TEST(test_PenaltyFarklingPhase_ZeroCeilingSafety);
    RUN_TEST(test_PenaltyFarklingPhase_InputSpamming);
    RUN_TEST(test_PenaltyFarklingPhase_ManualAdvance);
}
