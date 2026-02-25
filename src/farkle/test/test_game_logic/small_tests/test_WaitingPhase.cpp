#include "test_WaitingPhase.h"
#include "Game.h"
#include "phases/WaitingPhase.h"
#include "phases/BankingPhase.h"
#include "phases/FarklingPhase.h"
#include "../test_utils.h"
#include <unity.h>

// Verifies that the atRiskScore correctly accumulates when score buttons are pressed.
void test_WaitingPhase_ScoreAccumulation() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Ensure atRiskScore is initially 0
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);

    // Simulate pressing PLUS_500 and PLUS_100 (Total 600)
    simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::PLUS_100);

    // Removed rotation test as per request "Waiting phase can entirely ignore scrolling, please update tests as well to not allow this"
    // So we verify that rotation does NOTHING
    simulateRotation(game, 2); // 2 clicks

    // Verify atRiskScore is 600 (not 700)
    TEST_ASSERT_EQUAL_INT(600, game.state.atRiskScore);
}

// Verifies that the atRiskScore is cleared when the CLEAR button is pressed.
void test_WaitingPhase_ScoreCorrection() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Add some score
    simulateButtonPress(game, ButtonAction::PLUS_500, 2);
    TEST_ASSERT_EQUAL_INT(1000, game.state.atRiskScore);

    // Simulate pressing CLEAR
    simulateButtonPress(game, ButtonAction::CLEAR);

    // Verify atRiskScore is 0
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
}

// Verifies that the game transitions to the BankingPhase when the BANK button is pressed.
void test_WaitingPhase_TransitionToBanking() {
    Game game;
    setupGameWithPlayers(game, 4); // Initialize the game

    // Ensure initial state is WaitingPhase
    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);

    // Add some score to enable the BANK transition
    simulateButtonPress(game, ButtonAction::PLUS_500);

    // Simulate pressing the BANK button
    simulateButtonPress(game, ButtonAction::BANK);

    // Verify the state has transitioned to BankingPhase
    TEST_ASSERT_EQUAL_PTR(game.getPhase<BankingPhase>(), game.currentPhase);
}

// Verifies that the game transitions to the FarklingPhase when the FARKLE button is pressed.
void test_WaitingPhase_TransitionToFarkling() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Ensure initial state is WaitingPhase
    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);

    // Simulate pressing the FARKLE button
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Verify the state has transitioned to FarklingPhase
    TEST_ASSERT_EQUAL_PTR(game.getPhase<FarklingPhase>(), game.currentPhase);
}

// Verifies that PenaltyFarklingPhase IS triggered if the player has 3 consecutive farkles.
void test_WaitingPhase_TransitionToPenaltyFarkling() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.players[0].farkle_count = 2;
    game.state.players[0].score = 1000;

    simulateButtonPress(game, ButtonAction::FARKLE);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);
}

// Verifies that the Competition Score display blinks during the final round in WaitingPhase.
void test_WaitingPhase_FinalRoundBlinking() {
    Game game;
    setupGameWithPlayers(game, 2);
    game.state.finalRoundTriggered = true;
    game.currentPhase = game.getPhase<WaitingPhase>();

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.oled);
    game.currentPhase->display(game.state, displays);

    TEST_ASSERT_TRUE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::COMPETITION_SCORE]);
}

void run_waiting_phase_tests() {
    RUN_TEST(test_WaitingPhase_ScoreAccumulation);
    RUN_TEST(test_WaitingPhase_ScoreCorrection);
    RUN_TEST(test_WaitingPhase_TransitionToBanking);
    RUN_TEST(test_WaitingPhase_TransitionToFarkling);
    RUN_TEST(test_WaitingPhase_TransitionToPenaltyFarkling);
    RUN_TEST(test_WaitingPhase_FinalRoundBlinking);
}
