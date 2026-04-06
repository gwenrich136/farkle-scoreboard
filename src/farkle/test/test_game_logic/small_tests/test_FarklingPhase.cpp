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
    setupGameWithPlayers(game, 4);
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
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 50;
    game.currentPhase = game.getPhase<FarklingPhase>();
    game.currentPhase->onEnter(game.state);

    simulateNoAction(game, 1000);

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
}

// Verifies that button presses are ignored while the farkling animation is in progress.
void test_FarklingPhase_InputSpamming() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 500;
    game.currentPhase = game.getPhase<FarklingPhase>();
    game.currentPhase->onEnter(game.state);

    simulateButtonPress(game, ButtonAction::BANK);
    simulateButtonPress(game, ButtonAction::CLEAR);
    simulateButtonPress(game, ButtonAction::PLUS_500);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<FarklingPhase>(), game.currentPhase);
}

// Verifies that farkle_count does NOT increment if the player has 0 points.
void test_FarklingPhase_NoHarmNoFoul_NoIncrement() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.players[0].score = 0;
    game.state.players[0].farkle_count = 0;

    game.currentPhase = game.getPhase<FarklingPhase>();
    game.currentPhase->onEnter(game.state);

    TEST_ASSERT_EQUAL_INT(0, game.state.players[0].farkle_count);
}

// Verifies that farkle_count DOES increment if the player has points.
void test_FarklingPhase_IncrementWithPoints() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.players[0].score = 100;
    game.state.players[0].farkle_count = 0;

    game.currentPhase = game.getPhase<FarklingPhase>();
    game.currentPhase->onEnter(game.state);

    TEST_ASSERT_EQUAL_INT(1, game.state.players[0].farkle_count);
}

// Verifies that a button press transitions to the WaitingPhase after the animation is complete.
void test_FarklingPhase_ManualAdvance() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 0;
    game.currentPhase = game.getPhase<FarklingPhase>();
    game.currentPhase->onEnter(game.state);
    game.state.currentPlayerIndex = 0;

    simulateNoAction(game);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<FarklingPhase>(), game.currentPhase);

    simulateButtonPress(game, ButtonAction::CLEAR);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
}

// Verifies that the Competition Score display blinks during the final round in FarklingPhase.
void test_FarklingPhase_FinalRoundBlinking() {
    Game game;
    setupGameWithPlayers(game, 2);
    game.state.finalRoundTriggered = true;
    game.currentPhase = game.getPhase<FarklingPhase>();

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay);
    game.currentPhase->display(game.state, displays);

    TEST_ASSERT_TRUE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::COMPETITION_SCORE]);
}

// Verifies that the LedProgressGrid receives the potential score as the base score, and NO blinking score during the FarklingPhase.
void test_FarklingPhase_GridAnimationScores() {
    Game game;
    setupGameWithPlayers(game, 2);
    game.state.players[0].score = 1000;
    game.state.atRiskScore = 500;
    game.currentPhase = game.getPhase<FarklingPhase>();

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay);
    game.currentPhase->display(game.state, displays);

    // It should display the player's potential score (1500) falling back to base.
    TEST_ASSERT_EQUAL_INT(1500, game.grid.captured_scores[0]);
    // The blinking score should be 0.
    TEST_ASSERT_EQUAL_INT(0, game.grid.captured_blinkingScore);
}

void run_farkling_phase_tests() {
    RUN_TEST(test_FarklingPhase_AnimationMath);
    RUN_TEST(test_FarklingPhase_ZeroFloorSafety);
    RUN_TEST(test_FarklingPhase_InputSpamming);
    RUN_TEST(test_FarklingPhase_ManualAdvance);
    RUN_TEST(test_FarklingPhase_NoHarmNoFoul_NoIncrement);
    RUN_TEST(test_FarklingPhase_IncrementWithPoints);
    RUN_TEST(test_FarklingPhase_FinalRoundBlinking);
    RUN_TEST(test_FarklingPhase_GridAnimationScores);
}
