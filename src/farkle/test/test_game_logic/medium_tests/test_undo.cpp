#include "test_undo.h"
#include "Game.h"
#include "MemoryCard.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Helper: configure the mock to return a successful undo for player 1 with previousScore=500
static void configureMockUndo(Game& game, uint8_t playerIdx, int prevScore, uint8_t prevFarkles) {
    game.getMemoryCard().mock_undoLastTurn_result = {true, playerIdx, prevScore, prevFarkles};
}

// Verifies that when undoLastTurn succeeds, the current player index steps back
// and the previous player's score and farkle count are restored.
void test_Undo_StepsBackAndRestoresScore() {
    Game game;
    setupGameWithPlayers(game, 2);

    // Player 0 takes a turn and banks 1000
    game.state.players[0].score = 1000;
    game.state.currentPlayerIndex = 1; // It's now player 1's turn

    // Configure the mock to simulate undoing player 0's last turn
    configureMockUndo(game, /*playerIdx=*/0, /*prevScore=*/0, /*prevFarkles=*/0);

    simulateButtonPress(game, ButtonAction::UNDO);

    TEST_ASSERT_TRUE(game.getMemoryCard().mock_undoLastTurn_called);
    // Current player index should step back from 1 -> 0
    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);
    // Player 0's score should be restored to 0
    TEST_ASSERT_EQUAL_INT(0, game.state.players[0].score);
    // Player 0's farkle count should be restored to 0
    TEST_ASSERT_EQUAL_INT(0, game.state.players[0].farkle_count);
}

// Verifies that a failed undo (empty journal) does NOT change any game state.
void test_Undo_NoOpWhenJournalEmpty() {
    Game game;
    setupGameWithPlayers(game, 2);

    game.state.players[0].score = 500;
    game.state.currentPlayerIndex = 1;

    // Mock returns failure
    game.getMemoryCard().mock_undoLastTurn_result = {false, 0, 0, 0};

    simulateButtonPress(game, ButtonAction::UNDO);

    TEST_ASSERT_TRUE(game.getMemoryCard().mock_undoLastTurn_called);
    // State is unchanged
    TEST_ASSERT_EQUAL_INT(1, game.state.currentPlayerIndex);
    TEST_ASSERT_EQUAL_INT(500, game.state.players[0].score);
}

// Verifies that undo with player index 0 wraps correctly from 0 -> last player.
void test_Undo_WrapsAroundToLastPlayer() {
    Game game;
    setupGameWithPlayers(game, 3);

    // Player 0's turn, but we want to undo player 2's last turn
    game.state.currentPlayerIndex = 0;

    configureMockUndo(game, /*playerIdx=*/2, /*prevScore=*/300, /*prevFarkles=*/1);

    simulateButtonPress(game, ButtonAction::UNDO);

    // Current index should wrap 0 -> 2 (last player)
    TEST_ASSERT_EQUAL_INT(2, game.state.currentPlayerIndex);
    TEST_ASSERT_EQUAL_INT(300, game.state.players[2].score);
    TEST_ASSERT_EQUAL_INT(1, game.state.players[2].farkle_count);
}

// Verifies that undo also clears any pending atRiskScore.
void test_Undo_ClearsPendingAtRiskScore() {
    Game game;
    setupGameWithPlayers(game, 2);

    game.state.atRiskScore = 750; // Some pending score the current player had
    game.state.currentPlayerIndex = 1;

    configureMockUndo(game, 0, 0, 0);
    simulateButtonPress(game, ButtonAction::UNDO);

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
}

// Verifies that undo correctly restores a non-zero farkle count (e.g., player had 2 farkles).
void test_Undo_RestoresFarkleCount() {
    Game game;
    setupGameWithPlayers(game, 2);

    game.state.players[0].farkle_count = 0; // Currently 0 after the banked turn
    game.state.currentPlayerIndex = 1;

    // Undoing player 0's bank turn, where they had 2 farkles before banking
    configureMockUndo(game, 0, 500, 2);
    simulateButtonPress(game, ButtonAction::UNDO);

    TEST_ASSERT_EQUAL_INT(500, game.state.players[0].score);
    TEST_ASSERT_EQUAL_INT(2, game.state.players[0].farkle_count);
}

// Verifies that when an undo lands on the leading player, the competitor display
// defaults to showing rank 1 (2nd place), not rank 0 (the leader showing their own score).
void test_Undo_CompetitorDisplayUpdatedForLeader() {
    Game game;
    setupGameWithPlayers(game, 3);

    // Give player 0 the highest score so they are the leader at rank 0
    game.state.players[0].score = 5000;
    game.state.players[1].score = 3000;
    game.state.players[2].score = 1000;
    game.state.currentPlayerIndex = 1; // It's player 1's turn

    // The undo will step back to player 0 (the leader)
    configureMockUndo(game, /*playerIdx=*/0, /*prevScore=*/4000, /*prevFarkles=*/0);

    simulateButtonPress(game, ButtonAction::UNDO);

    // Player 0 is now current and is still the leader.
    // _recomputeLeaderboard must set currentCompetitorRank = 1 (not 0).
    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);
    TEST_ASSERT_EQUAL_INT(1, game.state.currentCompetitorRank);
}

void run_undo_tests() {
    RUN_TEST(test_Undo_StepsBackAndRestoresScore);
    RUN_TEST(test_Undo_NoOpWhenJournalEmpty);
    RUN_TEST(test_Undo_WrapsAroundToLastPlayer);
    RUN_TEST(test_Undo_ClearsPendingAtRiskScore);
    RUN_TEST(test_Undo_RestoresFarkleCount);
    RUN_TEST(test_Undo_CompetitorDisplayUpdatedForLeader);
}
