#include "test_conditional_at_risk_display.h"
#include "Game.h"
#include "../test_utils.h"
#include <unity.h>

// Verifies that during the selection phase, ScoreDisplay segments and FarkleWarningLights are explicitly cleared.
void test_DisplayLogic_PlayerSelection_DisplaysOff() {
    Game game;
    game.setup();

    // Loop once to trigger display()
    game.loop();

    // Verify all score segments are cleared (except competition which shows target score)
    TEST_ASSERT_TRUE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
    TEST_ASSERT_TRUE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE]);
    TEST_ASSERT_FALSE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::COMPETITION_SCORE]);
    TEST_ASSERT_EQUAL_INT(10000, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::COMPETITION_SCORE]);

    // Verify farkle lights are off
    TEST_ASSERT_EQUAL_INT(0, game.farkleLights.captured_state);
}

void test_DisplayLogic_WaitingPhase_ShowsZero() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Ensure we are in WaitingPhase
    game.state.atRiskScore = 0;
    game.loop();

    // In WaitingPhase, atRiskScore 0 should be printed as 0
    TEST_ASSERT_EQUAL_INT(0, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
    TEST_ASSERT_FALSE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
}

void test_DisplayLogic_BankingPhase_ClearsZero() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Enter BankingPhase
    game.state.atRiskScore = 100;
    simulateButtonPress(game, ButtonAction::BANK);

    // Wait for animation to finish
    while(game.state.atRiskScore > 0) {
        simulateNoAction(game);
    }

    // Now atRiskScore is 0, and we are still in BankingPhase (waiting for dismissal)
    game.loop();

    // In BankingPhase (default InGamePhase behavior), atRiskScore 0 should clear the display
    TEST_ASSERT_TRUE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
}

void test_DisplayLogic_PenaltyFarklingPhase_ClearsOnlyAtZero() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.players[0].farkle_count = 2;
    game.state.players[0].score = 1000;

    // Enter PenaltyFarklingPhase
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Stage 1: THE_PAIN (0-5s). Verify display is NOT cleared while score is -1000
    simulateNoAction(game, 1500);
    TEST_ASSERT_EQUAL_INT(-1000, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
    TEST_ASSERT_FALSE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
    TEST_ASSERT_TRUE(game.scoreDisplay.captured_blinks[ScoreDisplay::DisplayType::AT_RISK_SCORE]);

    // Advance past THE_PAIN (5 seconds total)
    simulateNoAction(game, 3501);

    // Stage 2: THE_DRAIN. Verify display is NOT cleared while score is still negative
    // We can just step once and check
    simulateNoAction(game, 100);
    if (game.state.atRiskScore < 0) {
        TEST_ASSERT_FALSE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
    }

    // Wait for THE_DRAIN animation to finish
    while(game.state.atRiskScore < 0) {
        simulateNoAction(game);
    }

    // Now atRiskScore is 0, still in PenaltyFarklingPhase (THE_AFTERMATH)
    game.loop();

    // Should be cleared now that it is 0
    TEST_ASSERT_TRUE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
}

void test_DisplayLogic_FarklingPhase_ClearsZero() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Enter FarklingPhase
    game.state.atRiskScore = 100;
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Wait for animation to finish
    while(game.state.atRiskScore > 0) {
        simulateNoAction(game);
    }

    // Now atRiskScore is 0, still in FarklingPhase
    game.loop();

    TEST_ASSERT_TRUE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
}

void test_DisplayLogic_InGamePhase_PassesAllFarkleCounts() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Set some farkle counts
    game.state.players[0].farkle_count = 0;
    game.state.players[1].farkle_count = 1;
    game.state.players[2].farkle_count = 2;
    game.state.players[3].farkle_count = 0;

    game.loop();

    // Verify farkle counts are passed to the mock
    TEST_ASSERT_EQUAL_INT(4, game.farkleLights.captured_farkleCounts.size());
    TEST_ASSERT_EQUAL_INT(0, game.farkleLights.captured_farkleCounts[0]);
    TEST_ASSERT_EQUAL_INT(1, game.farkleLights.captured_farkleCounts[1]);
    TEST_ASSERT_EQUAL_INT(2, game.farkleLights.captured_farkleCounts[2]);
    TEST_ASSERT_EQUAL_INT(0, game.farkleLights.captured_farkleCounts[3]);

    // Also verify player count and blinking player index (should be 0 for WaitingPhase)
    TEST_ASSERT_EQUAL_INT(4, game.farkleLights.captured_playerCount);
    TEST_ASSERT_EQUAL_INT(0, game.farkleLights.captured_blinkingPlayerIndex);
}

void test_DisplayLogic_BankingPhase_NoBlinking() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Add score so we can bank
    simulateButtonPress(game, ButtonAction::PLUS_500);
    simulateButtonPress(game, ButtonAction::PLUS_500);

    // Enter BankingPhase
    simulateButtonPress(game, ButtonAction::BANK);

    // During BankingPhase (InGamePhase subclass), blinkingPlayerIndex should be -1
    game.loop();

    TEST_ASSERT_EQUAL_INT(-1, game.farkleLights.captured_blinkingPlayerIndex);
}

void test_DisplayLogic_ScoreToggle() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Initial state: P1 has 1000 banked, 500 pending
    game.state.players[0].score = 1000;
    game.state.atRiskScore = 500;

    // Test BANKED mode (default)
    game.controlPad.setToggleState(ScoreDisplayMode::BANKED);
    game.loop();
    TEST_ASSERT_EQUAL_INT(1000, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE]);

    // Test PENDING mode
    game.controlPad.setToggleState(ScoreDisplayMode::PENDING);
    game.loop();
    TEST_ASSERT_EQUAL_INT(1500, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE]);

    // Simulate banking phase animation to ensure mode holds during animations
    simulateButtonPress(game, ButtonAction::BANK);

    // During banking, score moves from atRisk to banked
    // With PENDING mode, the total displayed should remain 1500 during the transfer
    game.loop();
    TEST_ASSERT_EQUAL_INT(1500, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE]);

    // Switch mid-animation back to BANKED
    game.controlPad.setToggleState(ScoreDisplayMode::BANKED);
    game.loop();

    // In BANKED mode mid-animation, it should show current banked score (which is now >= 1000 but < 1500)
    // NOTE: If loop executed quickly, currentBanked might still be 1000. So we check >= instead of >
    int currentBanked = game.state.players[0].score;
    TEST_ASSERT_EQUAL_INT(currentBanked, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE]);
    TEST_ASSERT_GREATER_OR_EQUAL(1000, currentBanked);
    TEST_ASSERT_LESS_THAN(1500, currentBanked);
}

void run_display_logic_tests() {
    RUN_TEST(test_DisplayLogic_PlayerSelection_DisplaysOff);
    RUN_TEST(test_DisplayLogic_WaitingPhase_ShowsZero);
    RUN_TEST(test_DisplayLogic_BankingPhase_ClearsZero);
    RUN_TEST(test_DisplayLogic_FarklingPhase_ClearsZero);
    RUN_TEST(test_DisplayLogic_PenaltyFarklingPhase_ClearsOnlyAtZero);
    RUN_TEST(test_DisplayLogic_InGamePhase_PassesAllFarkleCounts);
    RUN_TEST(test_DisplayLogic_BankingPhase_NoBlinking);
    RUN_TEST(test_DisplayLogic_ScoreToggle);
}
