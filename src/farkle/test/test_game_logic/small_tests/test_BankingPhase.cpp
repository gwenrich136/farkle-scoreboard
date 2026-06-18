#include "test_BankingPhase.h"
#include "Game.h"
#include "phases/BankingPhase.h"
#include "phases/WaitingPhase.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that the score animation correctly moves points from atRiskScore to the player's score.
void test_BankingPhase_AnimationMath() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 500;
    game.state.players[0].score = 0;
    game.currentPhase = game.getPhase<BankingPhase>();

    for (int i = 0; i < 101; i++) { // 101 * 10ms = 1010ms, which is enough for 500 points (needs 1000ms)
        simulateNoAction(game);
    }

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(500, game.state.players[0].score);
}

// Verifies that atRiskScore does not go negative when the points to be moved in one loop are greater than the remaining atRiskScore.
void test_BankingPhase_ZeroFloorSafety() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 50;
    game.state.players[0].score = 0;
    game.currentPhase = game.getPhase<BankingPhase>();

    simulateNoAction(game, 1000); // 1000ms should be enough to drain all 50 points in one loop

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(50, game.state.players[0].score);
}

// Verifies that button presses are ignored while the banking animation is in progress.
void test_BankingPhase_InputSpamming() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 500;
    game.currentPhase = game.getPhase<BankingPhase>();

    simulateButtonPress(game, ButtonAction::BANK);
    simulateButtonPress(game, ButtonAction::CLEAR);
    simulateButtonPress(game, ButtonAction::PLUS_500);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<BankingPhase>(), game.currentPhase);
}

// Verifies that when atRiskScore reaches 0, it transitions to EndOfTurnPhase.
void test_BankingPhase_TransitionsToEndOfTurn() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 0;
    game.currentPhase = game.getPhase<BankingPhase>();

    simulateNoAction(game);

    // As soon as atRiskScore is 0 and it updates, it transitions to EndOfTurnPhase
    TEST_ASSERT_EQUAL_PTR(game.getPhase<EndOfTurnPhase>(), game.currentPhase);
}

// Verifies that the farkle count is reset immediately upon entering the BankingPhase.
void test_BankingPhase_FarkleResetOnEnter() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.players[0].farkle_count = 2;

    // Explicitly enter the phase
    game.getPhase<BankingPhase>()->onEnter(game, game.state);

    TEST_ASSERT_EQUAL_INT(0, game.state.players[0].farkle_count);
}

// Verifies that the farkle warning lights are off during the banking animation.
void test_BankingPhase_LightsOffDuringAnimation() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.players[0].farkle_count = 2;
    game.currentPhase = game.getPhase<BankingPhase>();

    // Enter the phase and update display
    game.currentPhase->onEnter(game, game.state);

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay, game.soundPlayer);
    game.currentPhase->display(game.state, displays);

    // Captured state 0 means all lights are off
    TEST_ASSERT_EQUAL_INT(0, game.farkleLights.captured_state);
}

// Verifies that the Competition Score display blinks during the final round in BankingPhase.
void test_BankingPhase_FinalRoundBlinking() {
    Game game;
    setupGameWithPlayers(game, 2);
    game.state.finalRoundTriggered = true;
    game.currentPhase = game.getPhase<BankingPhase>();

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay, game.soundPlayer);
    game.currentPhase->display(game.state, displays);

    TEST_ASSERT_TRUE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::COMPETITION_SCORE]);
}

// Verifies that the LedProgressGrid receives the correct scores and NO blinking score during the BankingPhase.
void test_BankingPhase_GridAnimationScores() {
    Game game;
    setupGameWithPlayers(game, 2);
    game.state.players[0].score = 1000;
    game.state.atRiskScore = 500;
    game.currentPhase = game.getPhase<BankingPhase>();

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay, game.soundPlayer);
    game.currentPhase->display(game.state, displays);

    // It should display the player's base score (1000), not added to atRiskScore.
    TEST_ASSERT_EQUAL_INT(1000, game.grid.captured_scores[0]);
    // The blinking score should be 0.
    TEST_ASSERT_EQUAL_INT(0, game.grid.captured_blinkingScore);
}

// Verifies that the BankingPhase triggers SFX_BANKING and calls stop.
void test_BankingPhase_SoundEffectTriggered() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 500;
    game.currentPhase = game.getPhase<BankingPhase>();

    // Explicitly enter and then update to trigger the sound
    game.currentPhase->onEnter(game, game.state);

    GameInput input = {ButtonAction::NONE, 0, ScoreDisplayMode::BANKED};
    game.currentPhase->update(game, game.state, input, 10);

    TEST_ASSERT_TRUE(game.soundPlayer.play_called);
    TEST_ASSERT_EQUAL_INT(SFX_BANKING, game.soundPlayer.last_played_effect);
    TEST_ASSERT_FALSE(game.soundPlayer.stop_called);

    // Fast forward to end of animation
    for (int i = 0; i < 101; i++) {
        game.currentPhase->update(game, game.state, input, 10);
    }

    TEST_ASSERT_TRUE(game.soundPlayer.stop_called);
}

// Verifies that the score display updates correctly with toggle state.
void test_BankingPhase_ToggleStateDisplay() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 500;
    game.state.players[0].score = 1000;
    game.currentPhase = game.getPhase<BankingPhase>();

    // Test PENDING
    game.state.currentPlayerScoreMode = ScoreDisplayMode::PENDING;
    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay, game.soundPlayer);
    game.currentPhase->display(game.state, displays);
    TEST_ASSERT_EQUAL_INT(1500, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE]);

    // Test BANKED
    game.state.currentPlayerScoreMode = ScoreDisplayMode::BANKED;
    game.currentPhase->display(game.state, displays);
    TEST_ASSERT_EQUAL_INT(1000, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE]);
}

void run_banking_phase_tests() {
    RUN_TEST(test_BankingPhase_AnimationMath);
    RUN_TEST(test_BankingPhase_ZeroFloorSafety);
    RUN_TEST(test_BankingPhase_InputSpamming);
    RUN_TEST(test_BankingPhase_TransitionsToEndOfTurn);
    RUN_TEST(test_BankingPhase_FarkleResetOnEnter);
    RUN_TEST(test_BankingPhase_LightsOffDuringAnimation);
    RUN_TEST(test_BankingPhase_FinalRoundBlinking);
    RUN_TEST(test_BankingPhase_GridAnimationScores);
    RUN_TEST(test_BankingPhase_ToggleStateDisplay);
    RUN_TEST(test_BankingPhase_SoundEffectTriggered);
}
