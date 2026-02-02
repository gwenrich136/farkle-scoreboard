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

// Test function to verify the triple farkle penalty and reset behavior
void test_triple_farkle_penalty_and_reset() {
    Game game;
    game.setup();
    game.state.players[0].score = 2500; // Give player 1 some points

    // --- First Farkle ---
    game.controlPad.press(ButtonAction::FARKLE);
    game.loop();
    TEST_ASSERT_EQUAL_INT(1, game.state.players[0].farkle_count);
    game.controlPad.press(ButtonAction::BANK); // Dismiss
    game.loop();

    // --- Second Farkle ---
    game.state.currentPlayerIndex = 0; // Go back to Player 1
    game.controlPad.press(ButtonAction::FARKLE);
    game.loop();
    TEST_ASSERT_EQUAL_INT(2, game.state.players[0].farkle_count);
    game.controlPad.press(ButtonAction::BANK); // Dismiss
    game.loop();

    // --- Third Farkle - Trigger the penalty ---
    game.state.currentPlayerIndex = 0; // Go back to Player 1
    game.controlPad.press(ButtonAction::FARKLE);
    game.loop();
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);
    TEST_ASSERT_EQUAL_INT(-1000, game.state.atRiskScore); // atRiskScore is now -1000
    TEST_ASSERT_EQUAL_INT(2500, game.state.players[0].score); // Score is unchanged until animation
    TEST_ASSERT_EQUAL_INT(0, game.state.players[0].farkle_count); // Farkle count is reset

    // --- Run the penalty animation ---
    for (int i = 0; i < 200; ++i) {
        game.currentPhase->update(game, game.state, ButtonAction::NONE, 10);
    }
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(1500, game.state.players[0].score); // Final score is correct
}


void run_transition_tests() {
    RUN_TEST(test_transition_waiting_to_banking);
    RUN_TEST(test_transition_waiting_to_farkling);
    RUN_TEST(test_triple_farkle_penalty_and_reset);
}
