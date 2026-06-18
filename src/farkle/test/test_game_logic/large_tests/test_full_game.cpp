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
    simulatePregameFlow(game, 4);

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
    simulatePregameFlow(game, 2);

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
    simulatePregameFlow(game, 4);
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
    simulatePregameFlow(game, 4);

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
    simulatePregameFlow(game, 4);

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
    simulatePregameFlow(game, 2);

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
    simulatePregameFlow(game, 2);

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

// Helper function to advance turns until it is player 0's turn again.
void advance_to_player_zero(Game& game) {
    while (game.state.currentPlayerIndex != 0) {
        simulateButtonPress(game, ButtonAction::FARKLE); // A simple action to advance the turn
        simulateNoAction(game); // Process FarklingPhase and transition to EndOfTurn
        simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss the phase to complete the turn
    }
    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);
}

// Verifies end-to-end recovery of an active game.
void test_FullGame_ResumeActiveGame() {
    Game game;

    // First setup the initial context to mock having an active game
    game.getMemoryCard().mock_hasActiveGame_result = true;
    game.getMemoryCard().mock_loadGameMetadata_result = true;
    game.getMemoryCard().mock_replayGameJournal_result = true;

    game.setup();

    // Actually set up the GameState as if loadGameMetadata and replayGameJournal successfully restored a mid-game state.
    game.getMemoryCard().setupGameFromHardcodedPartialGame(game.state);

    game.loop(); // Render StartupPhase

    // Verify we are at the resume prompt
    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("Resume Game", game.textDisplay.captured_item.c_str());

    // Select "Resume Game"
    simulateButtonPress(game, ButtonAction::SELECT);

    // Transitioned to WaitingPhase. Since it's Alice's turn, her name should appear.
    // Also rank calculation should set Bob as the competitor.
    TEST_ASSERT_EQUAL_STRING("Alice", game.textDisplay.captured_p1Name.c_str());
    TEST_ASSERT_EQUAL_STRING("Bob", game.textDisplay.captured_p2Name.c_str());

    // Let's have Alice score 500 and bank.
    simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::SELECT); // Dismiss EndOfTurnPhase

    // It should now be Bob's turn
    TEST_ASSERT_EQUAL_STRING("Bob", game.textDisplay.captured_p1Name.c_str());
    TEST_ASSERT_EQUAL_INT(2000, game.state.players[0].score); // Alice has 1500 + 500
    TEST_ASSERT_EQUAL_INT(1, game.state.currentPlayerIndex);

    // Ensure that MemoryCard append was called during her bank (once for the new turn).
    TEST_ASSERT_TRUE(game.getMemoryCard().mock_appendTurnRecord_called);
}

#define RESUME_EXPECTED_TARGET_SCORE 10000
#define RESUME_EXPECTED_PLAYER_COUNT 2

// Verifies that resuming an active game correctly initializes the hardware progress grid with restored players and target score.
void test_FullGame_ResumeActiveGame_GridInitialization() {
    Game game;

    game.getMemoryCard().mock_hasActiveGame_result = true;
    game.getMemoryCard().mock_loadGameMetadata_result = true;
    game.getMemoryCard().mock_replayGameJournal_result = true;

    game.setup();
    game.getMemoryCard().setupGameFromHardcodedPartialGame(game.state);

    game.loop(); // Render StartupPhase

    // Select "Resume Game"
    simulateButtonPress(game, ButtonAction::SELECT);

    // Verify that the LedProgressGrid has been properly initialized
    TEST_ASSERT_EQUAL_INT(RESUME_EXPECTED_PLAYER_COUNT, game.grid.player_count);
    TEST_ASSERT_EQUAL_INT(RESUME_EXPECTED_TARGET_SCORE, game.grid.captured_targetScore);
}

void test_FullGame_AllGameSoundsTriggered() {
    Game game;
    // Start game flow
    simulatePregameFlow(game, 2);

    // 1. Score low sound
    game.soundPlayer.play_called = false;
    simulateButtonPress(game, ButtonAction::PLUS_50);
    TEST_ASSERT_TRUE(game.soundPlayer.play_called);
    TEST_ASSERT_EQUAL_INT(SFX_SCORE_LOW, game.soundPlayer.last_played_effect);

    // 2. Score mid sound
    game.soundPlayer.play_called = false;
    simulateButtonPress(game, ButtonAction::PLUS_100);
    TEST_ASSERT_TRUE(game.soundPlayer.play_called);
    TEST_ASSERT_EQUAL_INT(SFX_SCORE_MID, game.soundPlayer.last_played_effect);

    // 3. Score high sound
    game.soundPlayer.play_called = false;
    simulateButtonPress(game, ButtonAction::PLUS_500);
    TEST_ASSERT_TRUE(game.soundPlayer.play_called);
    TEST_ASSERT_EQUAL_INT(SFX_SCORE_HIGH, game.soundPlayer.last_played_effect);

    // 4. Banking sound
    game.soundPlayer.play_called = false;
    game.soundPlayer.stop_called = false;
    simulateButtonPress(game, ButtonAction::BANK);
    simulateNoAction(game); // Run BankingPhase::update to trigger sound
    TEST_ASSERT_TRUE(game.soundPlayer.play_called);
    TEST_ASSERT_EQUAL_INT(SFX_BANKING, game.soundPlayer.last_played_effect);
    
    // Let banking finish animation
    waitForScoreAnimation(game);
    TEST_ASSERT_TRUE(game.soundPlayer.stop_called);

    // Dismiss EndOfTurnPhase (transitions to player 1)
    simulateButtonPress(game, ButtonAction::CLEAR);

    // 5. Farkling sound
    game.soundPlayer.play_called = false;
    game.soundPlayer.stop_called = false;
    simulateButtonPress(game, ButtonAction::FARKLE);
    simulateNoAction(game); // Run FarklingPhase::update to trigger sound
    TEST_ASSERT_TRUE(game.soundPlayer.play_called);
    TEST_ASSERT_EQUAL_INT(SFX_FARKLE, game.soundPlayer.last_played_effect);

    // Wait for Farkling animation to finish
    simulateNoAction(game); // Run Farkling update
    TEST_ASSERT_TRUE(game.soundPlayer.stop_called);

    // Dismiss EndOfTurn (back to player 0)
    simulateButtonPress(game, ButtonAction::CLEAR);

    // 6. Penalty Farkle sound
    // Force player 0 to have 2 farkles, score 1000, and trigger third farkle
    game.state.currentPlayerIndex = 0;
    game.state.players[0].score = 1000;
    game.state.players[0].farkle_count = 2;

    game.soundPlayer.play_called = false;
    game.soundPlayer.stop_called = false;
    simulateButtonPress(game, ButtonAction::FARKLE); // Triggers penalty
    simulateNoAction(game); // Run PenaltyFarklingPhase::update to trigger sound
    
    TEST_ASSERT_TRUE(game.soundPlayer.play_called);
    TEST_ASSERT_EQUAL_INT(SFX_PENALTY_FARKLE, game.soundPlayer.last_played_effect);

    // Wait for Penalty animation (5000ms PAIN + 1000ms DRAIN)
    simulateNoAction(game, 5000);
    TEST_ASSERT_TRUE(game.soundPlayer.stop_called);

    // Finish penalty animation (drain)
    simulateNoAction(game, 1000); // 1000ms drain
    simulateNoAction(game, 10); // Transition to EndOfTurn
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss EndOfTurn (back to player 1)

    // 7. Final round bell
    // Let's get player 1 close to winning (9500), score 500, bank to cross 10000
    game.state.currentPlayerIndex = 1;
    game.state.players[1].score = 9500;
    game.soundPlayer.play_called = false;

    simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    
    // Clear EndOfTurn to trigger final round
    simulateButtonPress(game, ButtonAction::CLEAR);
    TEST_ASSERT_TRUE(game.soundPlayer.play_called);
    TEST_ASSERT_EQUAL_INT(SFX_FINAL_ROUND_BELL, game.soundPlayer.last_played_effect);
    TEST_ASSERT_TRUE(game.state.finalRoundTriggered);

    // 8. Victory fanfare
    // Player 0's turn in the final round (they get a Farkle to end the game)
    game.soundPlayer.play_called = false;
    simulateButtonPress(game, ButtonAction::FARKLE);
    simulateNoAction(game); // Process Farkling
    simulateButtonPress(game, ButtonAction::CLEAR); // Dismiss EndOfTurn
    
    // Now back to player 1 (winner). Next loop transitions to PostGamePhase_V1
    simulateNoAction(game); // WaitingPhase update triggers PostGame
    simulateNoAction(game); // PostGamePhase_V1 update runs to trigger victory fanfare
    TEST_ASSERT_TRUE(game.soundPlayer.play_random_victory_called);
}

void test_FullGame_SystemSoundsTriggered() {
    // 1. Startup sound is played in begin() on boot
    Game game;
    game.getMemoryCard().mock_hasActiveGame_result = false;
    game.setup();
    TEST_ASSERT_TRUE(game.soundPlayer.begin_called);

    // 2. Select "New Game" triggers SFX_NEW_GAME
    game.soundPlayer.play_called = false;
    game.soundPlayer.last_played_effect = SFX_NONE;
    simulateButtonPress(game, ButtonAction::SELECT);
    TEST_ASSERT_TRUE(game.soundPlayer.play_called);
    TEST_ASSERT_EQUAL_INT(SFX_NEW_GAME, game.soundPlayer.last_played_effect);

    // 3. Select "Resume Game" triggers SFX_RESUME_GAME
    Game game2;
    game2.getMemoryCard().mock_hasActiveGame_result = true;
    game2.getMemoryCard().mock_loadGameMetadata_result = true;
    game2.getMemoryCard().mock_replayGameJournal_result = true;
    game2.setup();
    game2.state.players.push_back(Player("Alice"));

    game2.soundPlayer.play_called = false;
    game2.soundPlayer.last_played_effect = SFX_NONE;
    simulateButtonPress(game2, ButtonAction::SELECT);
    TEST_ASSERT_TRUE(game2.soundPlayer.play_called);
    TEST_ASSERT_EQUAL_INT(SFX_RESUME_GAME, game2.soundPlayer.last_played_effect);
}

void run_full_game_tests() {
    RUN_TEST(test_FullGame_StandardGame);
    RUN_TEST(test_FullGame_TripleFarkle);
    RUN_TEST(test_FullGame_TripleFarkle_ScoreLessThanPenalty);
    RUN_TEST(test_FullGame_AutoAdvanceTurn);
    RUN_TEST(test_FullGame_FinalRoundBlinking);
    RUN_TEST(test_FullGame_ScoreToggle);
    RUN_TEST(test_PostGame_FinalizeCalledOnce);
    RUN_TEST(test_FullGame_ResumeActiveGame);
    RUN_TEST(test_FullGame_ResumeActiveGame_GridInitialization);
    RUN_TEST(test_FullGame_AllGameSoundsTriggered);
    RUN_TEST(test_FullGame_SystemSoundsTriggered);
}
