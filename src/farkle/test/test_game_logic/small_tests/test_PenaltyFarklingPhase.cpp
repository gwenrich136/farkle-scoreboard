#include "test_PenaltyFarklingPhase.h"
#include "Game.h"
#include "phases/PenaltyFarklingPhase.h"
#include "phases/WaitingPhase.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that the score animation correctly moves points from atRiskScore (negative) to 0
// and subtracts from the player's score, after the 5-second pause.
void test_PenaltyFarklingPhase_AnimationMath() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.atRiskScore = -1000;
    game.state.players[0].score = 2000;

    // 1. Stage: THE_PAIN (5000ms)
    // No points should move during this time.
    for (int i = 0; i < 490; i++) { // 4900ms
        simulateNoAction(game);
    }
    TEST_ASSERT_EQUAL_INT(-1000, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(2000, game.state.players[0].score);

    // 2. Stage: THE_DRAIN
    // Advance past 5000ms
    simulateNoAction(game, 200); // Now at 5100ms

    // Draining 1000 points at 1pt/ms takes another 1000ms.
    for (int i = 0; i < 150; i++) { // 1500ms more
        simulateNoAction(game);
    }

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(1000, game.state.players[0].score);
}

// Verifies that the blink parameter is correctly passed to the ScoreDisplay during THE_PAIN.
void test_PenaltyFarklingPhase_BlinkParameter() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.players[0].score = 1000; // Ensure atRiskScore is non-zero
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);

    simulateNoAction(game, 100); // Still in THE_PAIN
    TEST_ASSERT_TRUE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::AT_RISK_SCORE]);

    simulateNoAction(game, 5000); // Now in THE_DRAIN or AFTERMATH
    TEST_ASSERT_FALSE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
}

// Verifies that atRiskScore does not go above 0 (Zero Ceiling).
void test_PenaltyFarklingPhase_ZeroCeilingSafety() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.atRiskScore = -50;
    game.state.players[0].score = 2000;

    // Advance past THE_PAIN (5000ms)
    simulateNoAction(game, 5010);
    // Advance enough time to drain -50.
    simulateNoAction(game, 500);

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(1950, game.state.players[0].score);
}

// Verifies that button presses are ignored while the animation is in progress.
void test_PenaltyFarklingPhase_InputSpamming() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.atRiskScore = -500;

    simulateButtonPress(game, ButtonAction::BANK);
    simulateButtonPress(game, ButtonAction::CLEAR);
    simulateButtonPress(game, ButtonAction::PLUS_500);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);
}

// Verifies that a button press transitions to the WaitingPhase after the animation is complete.
void test_PenaltyFarklingPhase_ManualAdvance() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.atRiskScore = -100;
    
    // Advance past THE_PAIN (5000ms)
    simulateNoAction(game, 5010);
    // Advance past THE_DRAIN (100ms)
    simulateNoAction(game, 500);

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);

    simulateButtonPress(game, ButtonAction::CLEAR);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
}

// Verifies that the Competition Score display blinks during the final round in PenaltyFarklingPhase.
void test_PenaltyFarklingPhase_FinalRoundBlinking() {
    Game game;
    setupGameWithPlayers(game, 2);
    game.state.finalRoundTriggered = true;
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay);
    game.currentPhase->display(game.state, displays);

    TEST_ASSERT_TRUE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::COMPETITION_SCORE]);
}

// Verifies that the LedProgressGrid receives the correct scores and NO blinking score during the PenaltyFarklingPhase.
void test_PenaltyFarklingPhase_GridAnimationScores() {
    Game game;
    setupGameWithPlayers(game, 2);
    game.state.players[0].score = 1000;
    game.currentPhase = game.getPhase<PenaltyFarklingPhase>();
    game.currentPhase->onEnter(game.state);

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay);
    game.currentPhase->display(game.state, displays);

    // It should display the player's base score.
    TEST_ASSERT_EQUAL_INT(1000, game.grid.captured_scores[0]);
    // The blinking score should be 0.
    TEST_ASSERT_EQUAL_INT(0, game.grid.captured_blinkingScore);
}

void run_penalty_farkling_phase_tests() {
    RUN_TEST(test_PenaltyFarklingPhase_AnimationMath);
    RUN_TEST(test_PenaltyFarklingPhase_BlinkParameter);
    RUN_TEST(test_PenaltyFarklingPhase_ZeroCeilingSafety);
    RUN_TEST(test_PenaltyFarklingPhase_InputSpamming);
    RUN_TEST(test_PenaltyFarklingPhase_ManualAdvance);
    RUN_TEST(test_PenaltyFarklingPhase_FinalRoundBlinking);
    RUN_TEST(test_PenaltyFarklingPhase_GridAnimationScores);
}
