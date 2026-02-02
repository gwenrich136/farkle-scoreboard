#include "test_full_game.h"
#include "Game.h"
#include "phases/PostGamePhase_V1.h"
#include <unity.h>
#include "Arduino.h"

// Simulates a full game where players take turns scoring until one player reaches the target score, triggering the final round.
void test_FullGame_StandardGame() {
    Game game;
    game.setup();

    int turn = 0;
    while (game.currentPhase != game.getPhase<PostGamePhase_V1>() && turn < 100) {
        game.controlPad.press(ButtonAction::UP_1000);
        advance_millis(10);
        game.loop();
        game.controlPad.press(ButtonAction::RIGHT_500);
        advance_millis(10);
        game.loop();
        game.controlPad.press(ButtonAction::BANK);

        while (game.state.atRiskScore > 0) {
            advance_millis(10);
            game.loop();
        }

        // Press a button to advance the turn
        game.controlPad.press(ButtonAction::BANK);
        advance_millis(10);
        game.loop();

        turn++;
    }

    TEST_ASSERT_LESS_THAN(100, turn); // Game should end in a reasonable number of turns
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PostGamePhase_V1>(), game.currentPhase);
}

// Test function to verify the triple farkle penalty and reset behavior
void test_FullGame_TripleFarkle() {
    Game game;
    game.setup();
    game.state.players[0].score = 2500; // Give player 1 some points

    // --- First Farkle ---
    game.controlPad.press(ButtonAction::FARKLE);
    advance_millis(10);
    game.loop();
    TEST_ASSERT_EQUAL_INT(1, game.state.players[0].farkle_count);
    game.controlPad.press(ButtonAction::BANK); // Dismiss
    advance_millis(10);
    game.loop();
    advance_to_player_zero(game); // Go back to Player 1

    // --- Second Farkle ---
    game.controlPad.press(ButtonAction::FARKLE);
    advance_millis(10);
    game.loop();
    TEST_ASSERT_EQUAL_INT(2, game.state.players[0].farkle_count);
    game.controlPad.press(ButtonAction::BANK); // Dismiss
    advance_millis(10);
    game.loop();
    advance_to_player_zero(game); // Go back to Player 1

    // --- Player 1 banks, which should reset their farkle count ---
    game.controlPad.press(ButtonAction::UP_1000);
    advance_millis(10);
    game.loop();
    game.controlPad.press(ButtonAction::BANK);
    advance_millis(10);
    game.loop();
    while (game.state.atRiskScore > 0) {
        advance_millis(10);
        game.loop(); // Finish animation
    }
    game.controlPad.press(ButtonAction::BANK); // Dismiss
    advance_millis(10);
    game.loop();
    TEST_ASSERT_EQUAL_INT(0, game.state.players[0].farkle_count); // Verify reset
    advance_to_player_zero(game); // Go back to Player 1

    // --- Re-establish 2-farkle streak ---
    game.controlPad.press(ButtonAction::FARKLE);
    game.loop();
    game.controlPad.press(ButtonAction::BANK);
    game.loop();
    advance_to_player_zero(game);
    game.controlPad.press(ButtonAction::FARKLE);
    game.loop();
    game.controlPad.press(ButtonAction::BANK);
    game.loop();
    advance_to_player_zero(game);
    TEST_ASSERT_EQUAL_INT(2, game.state.players[0].farkle_count);

    // --- Third Farkle - Trigger the penalty ---
    game.controlPad.press(ButtonAction::FARKLE);
    advance_millis(10);
    game.loop();
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);
    // Initial score 2500, +1000 banked, -1000 penalty = 2500
    TEST_ASSERT_EQUAL_INT(2500, game.state.players[0].score);
    TEST_ASSERT_EQUAL_INT(0, game.state.players[0].farkle_count); // Should be reset
}

void run_full_game_tests() {
    RUN_TEST(test_FullGame_StandardGame);
    RUN_TEST(test_FullGame_TripleFarkle);
}

// Helper function to advance turns until it is player 0's turn again.
void advance_to_player_zero(Game& game) {
    while (game.state.currentPlayerIndex != 0) {
        game.controlPad.press(ButtonAction::FARKLE); // A simple action to advance the turn
        advance_millis(10);
        game.loop();
        game.controlPad.press(ButtonAction::BANK); // Dismiss the phase to complete the turn
        advance_millis(10);
        game.loop();
    }
    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);
}