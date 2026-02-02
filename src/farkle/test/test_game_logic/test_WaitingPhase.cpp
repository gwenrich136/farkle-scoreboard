#include "test_WaitingPhase.h"
#include "Game.h"
#include "phases/WaitingPhase.h"
#include "phases/BankingPhase.h"
#include "phases/FarklingPhase.h"
#include <unity.h>

// Verifies that the atRiskScore correctly accumulates when score buttons are pressed.
void test_WaitingPhase_ScoreAccumulation() {
    Game game;
    game.setup();

    // Ensure atRiskScore is initially 0
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);

    // Simulate pressing UP_1000 and RIGHT_500
    game.controlPad.press(ButtonAction::UP_1000);
    game.loop();
    game.controlPad.press(ButtonAction::RIGHT_500);
    game.loop();

    // Verify atRiskScore is 1500
    TEST_ASSERT_EQUAL_INT(1500, game.state.atRiskScore);
}

// Verifies that the atRiskScore is cleared when the CLEAR button is pressed.
void test_WaitingPhase_ScoreCorrection() {
    Game game;
    game.setup();

    // Add some score
    game.controlPad.press(ButtonAction::UP_1000);
    game.loop();
    TEST_ASSERT_EQUAL_INT(1000, game.state.atRiskScore);

    // Simulate pressing CLEAR
    game.controlPad.press(ButtonAction::CLEAR);
    game.loop();

    // Verify atRiskScore is 0
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
}

// Verifies that the game transitions to the BankingPhase when the BANK button is pressed.
void test_WaitingPhase_TransitionToBanking() {
    Game game;
    game.setup(); // Initialize the game

    // Ensure initial state is WaitingPhase
    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);

    // Add some score to enable the BANK transition
    game.controlPad.press(ButtonAction::UP_1000);
    game.loop();

    // Simulate pressing the BANK button
    game.controlPad.press(ButtonAction::BANK);
    game.loop(); // Run one loop to process the input

    // Verify the state has transitioned to BankingPhase
    TEST_ASSERT_EQUAL_PTR(game.getPhase<BankingPhase>(), game.currentPhase);
}

// Verifies that the game transitions to the FarklingPhase when the FARKLE button is pressed.
void test_WaitingPhase_TransitionToFarkling() {
    Game game;
    game.setup();

    // Ensure initial state is WaitingPhase
    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);

    // Simulate pressing the FARKLE button
    game.controlPad.press(ButtonAction::FARKLE);
    game.loop();

    // Verify the state has transitioned to FarklingPhase
    TEST_ASSERT_EQUAL_PTR(game.getPhase<FarklingPhase>(), game.currentPhase);
}

void run_waiting_phase_tests() {
    RUN_TEST(test_WaitingPhase_ScoreAccumulation);
    RUN_TEST(test_WaitingPhase_ScoreCorrection);
    RUN_TEST(test_WaitingPhase_TransitionToBanking);
    RUN_TEST(test_WaitingPhase_TransitionToFarkling);
}
