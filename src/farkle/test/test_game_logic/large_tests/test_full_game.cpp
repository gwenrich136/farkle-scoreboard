#include "test_full_game.h"
#include "Game.h"
#include "phases/PostGamePhase_V1.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Forward declaration of helper
void advance_to_player_zero(Game& game);

// Simulates a full game where players take turns scoring until one player reaches the target score, triggering the final round.
void test_FullGame_StandardGame() {
    Game game;
    setupGameWithPlayers(game, 4);

    int turn = 0;
    while (game.currentPhase != game.getPhase<PostGamePhase_V1>() && turn < 100) {
        // simulateScore(1500) replaced with button presses
        simulateButtonPress(game, ButtonAction::PLUS_500);
        simulateButtonPress(game, ButtonAction::PLUS_500);
        simulateButtonPress(game, ButtonAction::PLUS_500);
        
        // Start banking
        simulateButtonPress(game, ButtonAction::BANK);

        while (game.state.atRiskScore > 0) {
            simulateNoAction(game);
        }

        // Press a button to advance the turn
        simulateButtonPress(game, ButtonAction::CLEAR);

        turn++;
    }

    TEST_ASSERT_LESS_THAN(100, turn); // Game should end in a reasonable number of turns
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PostGamePhase_V1>(), game.currentPhase);
}

// Test function to verify the triple farkle penalty and reset behavior
void test_FullGame_TripleFarkle() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.players[0].score = 2500; // Give player 1 some points

    // --- First Farkle ---
    simulateButtonPress(game, ButtonAction::FARKLE);
    TEST_ASSERT_EQUAL_INT(1, game.state.players[0].farkle_count);
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss

    // --- Second Farkle ---
    game.state.currentPlayerIndex = 0; // Go back to Player 1
    simulateButtonPress(game, ButtonAction::FARKLE);
    TEST_ASSERT_EQUAL_INT(2, game.state.players[0].farkle_count);
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss

    // --- Third Farkle - Trigger the penalty ---
    game.state.currentPlayerIndex = 0; // Go back to Player 1
    simulateButtonPress(game, ButtonAction::FARKLE);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);
    TEST_ASSERT_EQUAL_INT(-1000, game.state.atRiskScore); // atRiskScore is now -1000
    TEST_ASSERT_EQUAL_INT(2500, game.state.players[0].score); // Score is unchanged until animation
    TEST_ASSERT_EQUAL_INT(0, game.state.players[0].farkle_count); // Farkle count is reset

    // --- Run the penalty animation ---
    // Needs to cover 3000ms PAIN + 1000ms DRAIN
    GameInput noInput;
    noInput.action = ButtonAction::NONE;
    noInput.rotationDelta = 0;
    for (int i = 0; i < 450; ++i) {
        game.currentPhase->update(game, game.state, noInput, 10);
    }
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
    TEST_ASSERT_EQUAL_INT(1500, game.state.players[0].score); // Final score is correct
}

void test_FullGame_TripleFarkle_ScoreLessThanPenalty() {
    Game game;
    setupGameWithPlayers(game, 4);

    // --- Player 0 scores 500 points ---
    simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss
    TEST_ASSERT_EQUAL_INT(500, game.state.players[0].score);

    // --- First Farkle ---
    advance_to_player_zero(game);
    simulateButtonPress(game, ButtonAction::FARKLE);
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss

    // --- Second Farkle ---
    advance_to_player_zero(game);
    simulateButtonPress(game, ButtonAction::FARKLE);
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss

    // --- Third Farkle - Trigger penalty ---
    advance_to_player_zero(game);
    simulateButtonPress(game, ButtonAction::FARKLE);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);

    // --- Run the penalty animation ---
    waitForScoreAnimation(game);
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);

    // --- Dismiss the penalty phase ---
    simulateButtonPress(game, ButtonAction::CLEAR);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);

    // Score should be 0, not negative
    TEST_ASSERT_EQUAL_INT(0, game.state.players[0].score);
}

// Verifies that the Competition Score display begins blinking as soon as the final round is triggered.
void test_FullGame_FinalRoundBlinking() {
    Game game;
    setupGameWithPlayers(game, 2);

    // Player 0 is about to reach the target score (10,000)
    game.state.players[0].score = 9500;
    game.state.currentPlayerIndex = 0;

    // Verify it is NOT blinking yet
    game.loop();
    TEST_ASSERT_FALSE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::COMPETITION_SCORE]);

    // Player 0 scores enough to cross the target score
    simulateButtonPress(game, ButtonAction::PLUS_500); // 9500 + 500 = 10000
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);

    // Verify it is NOT blinking yet because finalRoundTriggered is set after banking is dismissed
    game.loop();
    TEST_ASSERT_FALSE(game.state.finalRoundTriggered);
    TEST_ASSERT_FALSE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::COMPETITION_SCORE]);

    // Dismiss the banking phase
    simulateButtonPress(game, ButtonAction::CLEAR);

    // Now finalRoundTriggered should be true
    TEST_ASSERT_TRUE(game.state.finalRoundTriggered);

    // Run one loop to update displays
    game.loop();

    // Verify it IS blinking now
    TEST_ASSERT_TRUE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::COMPETITION_SCORE]);
}


void run_full_game_tests() {
    RUN_TEST(test_FullGame_StandardGame);
    RUN_TEST(test_FullGame_TripleFarkle);
    RUN_TEST(test_FullGame_TripleFarkle_ScoreLessThanPenalty);
    RUN_TEST(test_FullGame_FinalRoundBlinking);
}

// Helper function to advance turns until it is player 0's turn again.
void advance_to_player_zero(Game& game) {
    while (game.state.currentPlayerIndex != 0) {
        simulateButtonPress(game, ButtonAction::FARKLE); // A simple action to advance the turn
        simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss the phase to complete the turn
    }
    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);
}
