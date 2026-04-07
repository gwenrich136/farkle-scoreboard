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

    // Rotation should not modify atRiskScore
    simulateRotation(game, 2); // 2 clicks

    // Verify atRiskScore is 600
    TEST_ASSERT_EQUAL_INT(600, game.state.atRiskScore);
}

// Verifies leaderboard scrolling behavior
void test_WaitingPhase_LeaderboardScrolling() {
    Game game;
    setupGameWithPlayers(game, 4);

    // P0: 1000, P1: 2000, P2: 500, P3: 3000
    // Expected rank: P3 (3000), P1 (2000), P0 (1000), P2 (500)
    game.state.players[0].score = 1000;
    game.state.players[1].score = 2000;
    game.state.players[2].score = 500;
    game.state.players[3].score = 3000;

    game.state.currentPlayerIndex = 0; // P0 is playing
    game.currentPhase = game.getPhase<WaitingPhase>();
    game.currentPhase->onEnter(game.state);

    // Initial state: P0 is current. Top competitor is P3.
    TEST_ASSERT_EQUAL_INT(4, game.state.rankedPlayerIndices.size());
    TEST_ASSERT_EQUAL_INT(3, game.state.rankedPlayerIndices[0]);
    TEST_ASSERT_EQUAL_INT(1, game.state.rankedPlayerIndices[1]);
    TEST_ASSERT_EQUAL_INT(0, game.state.rankedPlayerIndices[2]);
    TEST_ASSERT_EQUAL_INT(2, game.state.rankedPlayerIndices[3]);

    // Competitor should initially point to rank 0 (P3)
    TEST_ASSERT_EQUAL_INT(0, game.state.currentCompetitorRank);

    // Rotate +1. Next is rank 1 (P1).
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_INT(1, game.state.currentCompetitorRank);

    // Rotate +1. Next is rank 2 (P0, which is current player!). Should skip to rank 3 (P2).
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_INT(3, game.state.currentCompetitorRank);

    // Rotate +1. Wraps around to rank 0 (P3).
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_INT(0, game.state.currentCompetitorRank);

    // Rotate -1. Next is rank 3 (P2).
    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_INT(3, game.state.currentCompetitorRank);
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

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay);
    game.currentPhase->display(game.state, displays);

    TEST_ASSERT_TRUE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::COMPETITION_SCORE]);
}

// Verifies that the LedProgressGrid receives the correct scores and YES blinking score during the WaitingPhase.
void test_WaitingPhase_GridAnimationScores() {
    Game game;
    setupGameWithPlayers(game, 2);
    game.state.players[0].score = 1000;
    game.state.atRiskScore = 500;
    game.currentPhase = game.getPhase<WaitingPhase>();

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay);
    game.currentPhase->display(game.state, displays);

    // It should display the player's base score.
    TEST_ASSERT_EQUAL_INT(1000, game.grid.captured_scores[0]);
    // The blinking score should be the atRiskScore.
    TEST_ASSERT_EQUAL_INT(500, game.grid.captured_blinkingScore);
}

// Verifies tiebreaker sorting when players have the same score
void test_WaitingPhase_TieBreakerSorting() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Give players the same score
    game.state.players[0].score = 4000;
    game.state.players[1].score = 4000;
    game.state.players[2].score = 4000;
    game.state.players[3].score = 4000;

    // Set current player to P1 (index 1)
    game.state.currentPlayerIndex = 1;
    game.currentPhase = game.getPhase<WaitingPhase>();
    game.currentPhase->onEnter(game.state);

    // Expected ranked list based on turnsAwayFromPlayer (ascending)
    // P1 (index 1) -> turnsAway = 0
    // P2 (index 2) -> turnsAway = 1
    // P3 (index 3) -> turnsAway = 2
    // P0 (index 0) -> turnsAway = 3
    TEST_ASSERT_EQUAL_INT(4, game.state.rankedPlayerIndices.size());
    TEST_ASSERT_EQUAL_INT(1, game.state.rankedPlayerIndices[0]);
    TEST_ASSERT_EQUAL_INT(2, game.state.rankedPlayerIndices[1]);
    TEST_ASSERT_EQUAL_INT(3, game.state.rankedPlayerIndices[2]);
    TEST_ASSERT_EQUAL_INT(0, game.state.rankedPlayerIndices[3]);

    // Now make P3 (index 3) the current player, but P1 and P2 have a different score.
    // P0: 3000, P1: 4000, P2: 4000, P3: 4000
    game.state.players[0].score = 3000;
    game.state.players[1].score = 4000;
    game.state.players[2].score = 4000;
    game.state.players[3].score = 4000;

    game.state.currentPlayerIndex = 3;
    game.currentPhase->onEnter(game.state);

    // Expected ranked list:
    // P3, P1, P2 all have 4000. P0 has 3000.
    // P3 turnsAway = 0 -> ranked 0
    // P1 turnsAway = 2 -> ranked 2 (wait, P0 is index 0. turnsAway(0)=1, turnsAway(1)=2, turnsAway(2)=3)
    // Let's list turnsAway from index 3:
    // P3 (idx 3) -> 0
    // P0 (idx 0) -> 1  (score 3000)
    // P1 (idx 1) -> 2  (score 4000)
    // P2 (idx 2) -> 3  (score 4000)
    // Rank logic:
    // 1st: P3 (4000, 0)
    // 2nd: P1 (4000, 2)
    // 3rd: P2 (4000, 3)
    // 4th: P0 (3000, 1) - score overrides turnsAway
    TEST_ASSERT_EQUAL_INT(4, game.state.rankedPlayerIndices.size());
    TEST_ASSERT_EQUAL_INT(3, game.state.rankedPlayerIndices[0]);
    TEST_ASSERT_EQUAL_INT(1, game.state.rankedPlayerIndices[1]);
    TEST_ASSERT_EQUAL_INT(2, game.state.rankedPlayerIndices[2]);
    TEST_ASSERT_EQUAL_INT(0, game.state.rankedPlayerIndices[3]);
}

void run_waiting_phase_tests() {
    RUN_TEST(test_WaitingPhase_TieBreakerSorting);
    RUN_TEST(test_WaitingPhase_ScoreAccumulation);
    RUN_TEST(test_WaitingPhase_LeaderboardScrolling);
    RUN_TEST(test_WaitingPhase_ScoreCorrection);
    RUN_TEST(test_WaitingPhase_TransitionToBanking);
    RUN_TEST(test_WaitingPhase_TransitionToFarkling);
    RUN_TEST(test_WaitingPhase_TransitionToPenaltyFarkling);
    RUN_TEST(test_WaitingPhase_FinalRoundBlinking);
    RUN_TEST(test_WaitingPhase_GridAnimationScores);
}
