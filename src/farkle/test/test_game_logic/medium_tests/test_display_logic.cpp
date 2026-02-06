#include "test_display_logic.h"
#include "Game.h"
#include "../test_utils.h"
#include <unity.h>

void test_DisplayLogic_WaitingPhase_ShowsZero() {
    Game game;
    game.setup();

    // Ensure we are in WaitingPhase
    game.state.atRiskScore = 0;
    game.loop();

    // In WaitingPhase, atRiskScore 0 should be printed as 0
    TEST_ASSERT_EQUAL_INT(0, game.scoreDisplay.captured_numbers[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
    TEST_ASSERT_FALSE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
}

void test_DisplayLogic_BankingPhase_ClearsZero() {
    Game game;
    game.setup();

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

void test_DisplayLogic_PenaltyFarklingPhase_ClearsZero() {
    Game game;
    game.setup();
    game.state.players[0].farkle_count = 2;
    game.state.players[0].score = 1000;

    // Enter PenaltyFarklingPhase
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Advance past THE_PAIN (3 seconds)
    simulateNoAction(game, 3001);

    // Wait for THE_DRAIN animation to finish
    while(game.state.atRiskScore < 0) {
        simulateNoAction(game);
    }

    // Now atRiskScore is 0, still in PenaltyFarklingPhase
    game.loop();

    TEST_ASSERT_TRUE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
}

void test_DisplayLogic_FarklingPhase_ClearsZero() {
    Game game;
    game.setup();

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

void run_display_logic_tests() {
    RUN_TEST(test_DisplayLogic_WaitingPhase_ShowsZero);
    RUN_TEST(test_DisplayLogic_BankingPhase_ClearsZero);
    RUN_TEST(test_DisplayLogic_FarklingPhase_ClearsZero);
    RUN_TEST(test_DisplayLogic_PenaltyFarklingPhase_ClearsZero);
}
