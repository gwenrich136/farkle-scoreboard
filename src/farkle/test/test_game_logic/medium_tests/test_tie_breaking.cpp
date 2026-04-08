#include "test_tie_breaking.h"
#include "Game.h"
#include "../test_utils.h"
#include "phases/PostGamePhase_V1.h"
#include <unity.h>

/**
 * Case 1
 * Three Players:
 * Player 1 scores 3000
 * Player 2 scores Farkles
 * Player 3 scores 3000
 * -> Player 1 should be pronounced the winner
 */
void test_TieBreaking_Case1() {
    Game game;
    setupGameWithPlayers(game, 3);
    game.setTargetScore(3000);

    // Player 1 scores 3000
    // 6 * 500 = 3000
    for(int i=0; i<6; ++i) simulateButtonPress(game, ButtonAction::PLUS_500);

    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::BANK); // Confirm banking

    TEST_ASSERT_EQUAL_INT(3000, game.state.players[0].score);
    TEST_ASSERT_TRUE(game.state.finalRoundTriggered);
    TEST_ASSERT_EQUAL_INT(1, game.state.currentPlayerIndex); // Player 2's turn

    // Player 2 Farkles
    simulateButtonPress(game, ButtonAction::FARKLE); // Enter FarklingPhase
    simulateNoAction(game); // Process FarklingPhase and transition to EndOfTurn
    simulateButtonPress(game, ButtonAction::FARKLE); // Confirm Farkle and advance
    TEST_ASSERT_EQUAL_INT(2, game.state.currentPlayerIndex); // Player 3's turn

    // Player 3 scores 3000
    for(int i=0; i<6; ++i) simulateButtonPress(game, ButtonAction::PLUS_500);

    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::BANK); // Confirm banking

    TEST_ASSERT_EQUAL_INT(3000, game.state.players[2].score);
    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex); // Back to Player 1

    // Verify ranked list right before game ends.
    // Player 1 (index 0): 3000, turnsAway=0
    // Player 2 (index 1): 0, turnsAway=1
    // Player 3 (index 2): 3000, turnsAway=2
    // Rank logic expects: Player 1, Player 3, Player 2
    TEST_ASSERT_EQUAL_INT(0, game.state.rankedPlayerIndices[0]);
    TEST_ASSERT_EQUAL_INT(2, game.state.rankedPlayerIndices[1]);
    TEST_ASSERT_EQUAL_INT(1, game.state.rankedPlayerIndices[2]);

    // One more loop to trigger the game end transition in WaitingPhase
    simulateNoAction(game);

    // Game should transition to PostGamePhase_V1
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PostGamePhase_V1>(), game.currentPhase);

    // Verify Player 1 wins
    TEST_ASSERT_EQUAL_STRING("Geewee WINS!", game.textDisplay.captured_message.c_str());
}

/**
 * Case 2
 * Three players:
 * Player 1 scores 3000
 * Player 2 scores 4000
 * Player 3 scores 4000
 * -> Player 2 should be pronounced winner
 */
void test_TieBreaking_Case2() {
    Game game;
    setupGameWithPlayers(game, 3);
    game.setTargetScore(3000);

    // Player 1 scores 3000
    for(int i=0; i<6; ++i) simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::BANK);

    // Player 2 scores 4000
    for(int i=0; i<8; ++i) simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::BANK);

    // Player 3 scores 4000
    for(int i=0; i<8; ++i) simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::BANK);

    // Current player is back to P1 (index 0)
    // P1 (index 0): 3000, turnsAway=0
    // P2 (index 1): 4000, turnsAway=1
    // P3 (index 2): 4000, turnsAway=2
    // Tiebreaker: P2 has turnsAway 1, P3 has turnsAway 2. P2 > P3.
    // Order: P2, P3, P1
    TEST_ASSERT_EQUAL_INT(1, game.state.rankedPlayerIndices[0]);
    TEST_ASSERT_EQUAL_INT(2, game.state.rankedPlayerIndices[1]);
    TEST_ASSERT_EQUAL_INT(0, game.state.rankedPlayerIndices[2]);

    // One more loop to trigger the game end transition in WaitingPhase
    simulateNoAction(game);

    // Game should transition to PostGamePhase_V1
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PostGamePhase_V1>(), game.currentPhase);

    // Verify Player 2 wins (Sammy)
    TEST_ASSERT_EQUAL_STRING("Sammy WINS!", game.textDisplay.captured_message.c_str());
}

/**
 * Case 3 (Edge Case)
 * Three players:
 * Player 1 scores 1000
 * Player 2 scores 3000 (Triggers final round)
 * Player 3 scores 3000
 * Player 1 scores 3000 (Extra turn)
 * -> Player 2 should be pronounced winner (Reached 3000 first)
 */
void test_TieBreaking_Case3() {
    Game game;
    setupGameWithPlayers(game, 3);
    game.setTargetScore(3000);

    // Player 1 scores 1000
    simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::BANK);

    // Player 2 scores 3000 (Triggers)
    for(int i=0; i<6; ++i) simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::BANK);

    TEST_ASSERT_TRUE(game.state.finalRoundTriggered);
    TEST_ASSERT_EQUAL_INT(2, game.state.currentPlayerIndex); // P3's turn

    // Player 3 scores 3000
    for(int i=0; i<6; ++i) simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::BANK);

    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex); // P1's extra turn

    // Player 1 scores 2000 more (Total 3000)
    for(int i=0; i<4; ++i) simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::BANK);
    waitForScoreAnimation(game);
    simulateButtonPress(game, ButtonAction::BANK);

    // Current player is P2 (index 1) since P1's turn ended.
    // Scores: P1=3000, P2=3000, P3=3000
    // Turns away from P2 (index 1):
    // P2 (idx 1): 0
    // P3 (idx 2): 1
    // P1 (idx 0): 2
    // Order: P2, P3, P1
    TEST_ASSERT_EQUAL_INT(1, game.state.rankedPlayerIndices[0]);
    TEST_ASSERT_EQUAL_INT(2, game.state.rankedPlayerIndices[1]);
    TEST_ASSERT_EQUAL_INT(0, game.state.rankedPlayerIndices[2]);

    // One more loop to trigger transition
    simulateNoAction(game);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<PostGamePhase_V1>(), game.currentPhase);

    // Verify Player 2 wins (Sammy) because they reached 3000 FIRST.
    TEST_ASSERT_EQUAL_STRING("Sammy WINS!", game.textDisplay.captured_message.c_str());
}

void run_tie_breaking_tests() {
    RUN_TEST(test_TieBreaking_Case1);
    RUN_TEST(test_TieBreaking_Case2);
    RUN_TEST(test_TieBreaking_Case3);
}
