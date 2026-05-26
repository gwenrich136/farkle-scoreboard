#include "test_EndOfTurnPhase.h"
#include "Game.h"
#include "phases/EndOfTurnPhase.h"
#include "phases/WaitingPhase.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that a button press advances the turn and transitions to WaitingPhase
void test_EndOfTurnPhase_ManualAdvance() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 0;
    game.state.currentPlayerIndex = 0;
    game.currentPhase = game.getPhase<EndOfTurnPhase>();
    game.currentPhase->onEnter(game.state);

    simulateNoAction(game);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<EndOfTurnPhase>(), game.currentPhase);
    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);

    simulateButtonPress(game, ButtonAction::CLEAR);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
    TEST_ASSERT_EQUAL_INT(1, game.state.currentPlayerIndex);
}

// Verifies that the phase correctly triggers final round if score condition met
void test_EndOfTurnPhase_FinalRoundTrigger() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 0;
    game.state.targetScore = 10000;
    game.state.players[0].score = 10000;
    game.state.currentPlayerIndex = 0;
    game.currentPhase = game.getPhase<EndOfTurnPhase>();
    game.currentPhase->onEnter(game.state);

    simulateButtonPress(game, ButtonAction::BANK);

    TEST_ASSERT_TRUE(game.state.finalRoundTriggered);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
}

// Verifies that the At-Risk display is cleared when the turn ends
void test_EndOfTurnPhase_DisplayClearsAtRisk() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.currentPhase = game.getPhase<EndOfTurnPhase>();

    Displays displays(game.scoreDisplay, game.grid, game.farkleLights, game.textDisplay, game.soundPlayer);
    game.currentPhase->display(game.state, displays);

    // ScoreDisplay mock captures the state of clearing the displays
    TEST_ASSERT_TRUE(game.scoreDisplay.cleared_displays[ScoreDisplay::DisplayType::AT_RISK_SCORE]);
}

// Verifies that a rotation advances the turn and transitions to WaitingPhase
void test_EndOfTurnPhase_RotationAdvance() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 0;
    game.state.currentPlayerIndex = 0;
    game.currentPhase = game.getPhase<EndOfTurnPhase>();
    game.currentPhase->onEnter(game.state);

    simulateNoAction(game);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<EndOfTurnPhase>(), game.currentPhase);
    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);

    simulateRotation(game, 1);

    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
    TEST_ASSERT_EQUAL_INT(1, game.state.currentPlayerIndex);
}

// Verifies that the phase properly waits and automatically advances after timeout
void test_EndOfTurnPhase_WaitWithoutInput() {
    Game game;
    setupGameWithPlayers(game, 4);
    game.state.atRiskScore = 0;
    game.currentPhase = game.getPhase<EndOfTurnPhase>();
    game.currentPhase->onEnter(game.state);

    int initialPlayerIndex = game.state.currentPlayerIndex;

    // Simulate 4 seconds (400 calls of simulateNoAction which advances by 10ms each, wait we can just pass time directly)
    for (int i = 0; i < 40; i++) {
        simulateNoAction(game, 100);
    }

    // Should still be in EndOfTurnPhase after 4 seconds
    TEST_ASSERT_EQUAL_PTR(game.getPhase<EndOfTurnPhase>(), game.currentPhase);
    TEST_ASSERT_EQUAL_INT(initialPlayerIndex, game.state.currentPlayerIndex);

    // Simulate 2 more seconds (total 6 seconds)
    for (int i = 0; i < 20; i++) {
        simulateNoAction(game, 100);
    }

    // Should have transitioned to WaitingPhase and advanced player index
    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
    TEST_ASSERT_NOT_EQUAL(initialPlayerIndex, game.state.currentPlayerIndex);
}

void run_end_of_turn_phase_tests() {
    RUN_TEST(test_EndOfTurnPhase_ManualAdvance);
    RUN_TEST(test_EndOfTurnPhase_RotationAdvance);
    RUN_TEST(test_EndOfTurnPhase_FinalRoundTrigger);
    RUN_TEST(test_EndOfTurnPhase_DisplayClearsAtRisk);
    RUN_TEST(test_EndOfTurnPhase_WaitWithoutInput);
}
