#include <unity.h>
#include "Game.h"

// Test function to verify the transition from Waiting to Banking
void test_transition_waiting_to_banking() {
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

// Test function to verify the transition from Waiting to Farkling
void test_transition_waiting_to_farkling() {
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

void run_transition_tests() {
    RUN_TEST(test_transition_waiting_to_banking);
    RUN_TEST(test_transition_waiting_to_farkling);
}
