#include "test_PenaltyFarklingPhase.h"
#include "Game.h"
#include "phases/PenaltyFarklingPhase.h"
#include "phases/WaitingPhase.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that the score animation correctly moves points from atRiskScore (negative) to 0
// and subtracts from the player's score, after the "PAIN" delay.
void test_PenaltyFarklingPhase_AnimationMath() {
    Game game;
    game.setup();
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.atRiskScore = -1000;
    game.state.players[0].score = 2000;

    // 1. Stage: THE_PAIN (3000ms)
    // No points should move during this time.
    for (int i = 0; i < 290; i++) { // 2900ms
        simulateNoAction(game);
    }
    TEST_ASSERT_EQUAL_INT(-1000, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(2000, game.state.players[0].score);

    // 2. Stage: THE_DRAIN
    // Transition to drain happens at 3000ms.
    // Draining 1000 points at 1pt/ms takes another 1000ms.
    for (int i = 0; i < 150; i++) { // Another 1500ms
        simulateNoAction(game);
    }

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(1000, game.state.players[0].score);
}

// Verifies that the atRiskScore display flashes during the THE_PAIN stage.
void test_PenaltyFarklingPhase_BlinkingEffect() {
    Game game;
    game.setup();
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.atRiskScore = -1000;

    // Initial state after onEnter and first loop (10ms)
    simulateNoAction(game, 10);
    TEST_ASSERT_EQUAL_INT(-1000, game.scoreDisplay.captured_numbers[0]);

    // Advance to 250ms -> should blink (clear)
    // We already advanced 10ms, so 240ms more.
    simulateNoAction(game, 240);
    TEST_ASSERT_EQUAL_INT(-99999, game.scoreDisplay.captured_numbers[0]);

    // Advance another 250ms -> should be solid again
    simulateNoAction(game, 250);
    TEST_ASSERT_EQUAL_INT(-1000, game.scoreDisplay.captured_numbers[0]);
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
    game.state.atRiskScore = -100;
    
    // Advance past THE_PAIN (3000ms)
    simulateNoAction(game, 3010);
    // Advance past THE_DRAIN (100ms)
    simulateNoAction(game, 500);

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);

    simulateButtonPress(game, ButtonAction::CLEAR);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
}

void run_penalty_farkling_phase_tests() {
    RUN_TEST(test_PenaltyFarklingPhase_AnimationMath);
    RUN_TEST(test_PenaltyFarklingPhase_BlinkingEffect);
    RUN_TEST(test_PenaltyFarklingPhase_InputSpamming);
    RUN_TEST(test_PenaltyFarklingPhase_ManualAdvance);
}
