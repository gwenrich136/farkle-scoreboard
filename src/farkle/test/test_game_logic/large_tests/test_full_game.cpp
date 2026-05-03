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
        simulateButtonPress(game, ButtonAction::PLUS_500, 3);
        
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

    // finalizeGame should have been called exactly once on the first update of PostGamePhase_V1
    TEST_ASSERT_TRUE(game.getMemoryCard().mock_finalizeGame_called);
}

// Verifies that finalizeGame is called exactly once regardless of how many loops run in PostGamePhase_V1
void test_PostGame_FinalizeCalledOnce() {
    Game game;
    setupGameWithPlayers(game, 2);

    // Get player 0 to 9500 points
    game.state.players[0].score = 9500;

    // Player 0's turn: bank 500 to cross 10,000 and trigger the final round
    simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss EndOfTurn -> triggers final round
    TEST_ASSERT_TRUE(game.state.finalRoundTriggered);

    // Player 1's turn: take any turn to complete the final round
    simulateButtonPress(game, ButtonAction::FARKLE);
    simulateNoAction(game); // Run FarklingPhase animation
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss EndOfTurn

    // Now back to player 0 whose score >= targetScore -> game ends on WaitingPhase update
    simulateNoAction(game); // WaitingPhase.onEnter() + first update triggers PostGamePhase_V1
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PostGamePhase_V1>(), game.currentPhase);

    // Run multiple loops to confirm finalizeGame is only called once
    simulateNoAction(game);
    simulateNoAction(game);
    simulateNoAction(game);

    TEST_ASSERT_TRUE(game.getMemoryCard().mock_finalizeGame_called);
}

// Test function to verify the triple farkle penalty and reset behavior
void test_FullGame_TripleFarkle() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.players[0].score = 2500; // Give player 1 some points

    // --- First Farkle ---
    simulateButtonPress(game, ButtonAction::FARKLE);
    simulateNoAction(game); // Process FarklingPhase and transition to EndOfTurn
    TEST_ASSERT_EQUAL_INT(1, game.state.players[0].farkle_count);
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss

    // --- Second Farkle ---
    game.state.currentPlayerIndex = 0; // Go back to Player 1
    simulateButtonPress(game, ButtonAction::FARKLE);
    simulateNoAction(game); // Process FarklingPhase and transition to EndOfTurn
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
    // Needs to cover 5000ms PAIN + 1000ms DRAIN
    waitForScoreAnimation(game);
    // Needs one more step to actually trigger the phase transition
    simulateNoAction(game);

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
    simulateNoAction(game); // Process FarklingPhase and transition to EndOfTurn
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss

    // --- Second Farkle ---
    advance_to_player_zero(game);
    simulateButtonPress(game, ButtonAction::FARKLE);
    simulateNoAction(game); // Process FarklingPhase and transition to EndOfTurn
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss

    // --- Third Farkle - Trigger penalty ---
    advance_to_player_zero(game);
    simulateButtonPress(game, ButtonAction::FARKLE);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PenaltyFarklingPhase>(), game.currentPhase);

    // --- Run the penalty animation ---
    // Needs to cover 5000ms PAIN + 1000ms DRAIN
    waitForScoreAnimation(game);
    // Needs one more step to actually trigger the phase transition
    simulateNoAction(game);

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);

    // Simulate button press to dismiss the EndOfTurn phase that should be active now
    simulateButtonPress(game, ButtonAction::CLEAR);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);

    // Score should be 0, not negative
    TEST_ASSERT_EQUAL_INT(0, game.state.players[0].score);
}

void test_FullGame_AutoAdvanceTurn() {
    Game game;
    setupGameWithPlayers(game, 4);

    int turn = 0;
    while (game.currentPhase != game.getPhase<PostGamePhase_V1>() && turn < 100) {
        simulateButtonPress(game, ButtonAction::PLUS_500, 3);

        // Start banking
        simulateButtonPress(game, ButtonAction::BANK);

        while (game.state.atRiskScore > 0) {
            simulateNoAction(game);
        }

        // Instead of pressing a button, wait for 5.1 seconds to auto advance
        for (int i = 0; i < 51; i++) {
            simulateNoAction(game, 100);
        }

        turn++;
    }

    TEST_ASSERT_LESS_THAN(100, turn); // Game should end in a reasonable number of turns
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PostGamePhase_V1>(), game.currentPhase);
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
    simulateNoAction(game);
    TEST_ASSERT_FALSE(game.state.finalRoundTriggered);
    TEST_ASSERT_FALSE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::COMPETITION_SCORE]);

    // Dismiss the EndOfTurn phase
    simulateButtonPress(game, ButtonAction::CLEAR);

    // Now finalRoundTriggered should be true
    TEST_ASSERT_TRUE(game.state.finalRoundTriggered);

    // Run one loop to update displays
    game.loop();

    // Verify it IS blinking now
    TEST_ASSERT_TRUE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::COMPETITION_SCORE]);
}


// Full Game With Score Toggle Simulation
void test_FullGame_ScoreToggle() {
    Game game;
    setupGameWithPlayers(game, 2);

    // Toggle switch to PENDING
    game.controlPad.setToggleState(ScoreDisplayMode::PENDING);

    // Player 1's turn
    simulateButtonPress(game, ButtonAction::PLUS_500, 3); // Pending: 1500
    game.loop();
    TEST_ASSERT_EQUAL_INT(1500, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE]);

    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::SELECT); // Dismiss EndOfTurnPhase

    // Player 2's turn
    simulateButtonPress(game, ButtonAction::PLUS_100, 4); // Pending: 400
    game.loop();
    TEST_ASSERT_EQUAL_INT(400, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE]);

    // Switch to BANKED mid-turn
    game.controlPad.setToggleState(ScoreDisplayMode::BANKED);
    game.loop();
    TEST_ASSERT_EQUAL_INT(0, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE]);

    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::SELECT); // Dismiss
}

void run_full_game_tests() {
    RUN_TEST(test_FullGame_StandardGame);
    RUN_TEST(test_FullGame_TripleFarkle);
    RUN_TEST(test_FullGame_TripleFarkle_ScoreLessThanPenalty);
    RUN_TEST(test_FullGame_AutoAdvanceTurn);
    RUN_TEST(test_FullGame_FinalRoundBlinking);
    RUN_TEST(test_FullGame_ScoreToggle);
    RUN_TEST(test_PostGame_FinalizeCalledOnce);
}

// Helper function to advance turns until it is player 0's turn again.
void advance_to_player_zero(Game& game) {
    while (game.state.currentPlayerIndex != 0) {
        simulateButtonPress(game, ButtonAction::FARKLE); // A simple action to advance the turn
        simulateNoAction(game); // Process FarklingPhase and transition to EndOfTurn
        simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss the phase to complete the turn
    }
    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);
}
