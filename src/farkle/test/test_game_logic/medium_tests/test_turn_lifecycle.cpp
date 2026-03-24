#include "test_turn_lifecycle.h"
#include "Game.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

/**
 * test_TurnLifecycle_FullSetupAndTurn
 *
 * This test simulates the full user journey from the initial Player Selection Phase
 * to the first turn in WaitingPhase. It verifies:
 * 1. The game starts in selection phase.
 * 2. Navigation through the name pool works.
 * 3. Adding specific players works and assigns them to the roster.
 * 4. The transition to WaitingPhase only happens after players are added.
 * 5. The correct player is active when the game starts.
 */
void test_TurnLifecycle_FullSetupAndTurn() {
    Game game;
    game.setup();
    game.loop();

    // 1. Target Score Selection
    TEST_ASSERT_EQUAL_STRING("Target Score", game.textDisplay.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("10,000", game.textDisplay.captured_item.c_str());

    // Transition to Player Selection
    simulateButtonPress(game, ButtonAction::SELECT);

    // 2. Initial selection state (Geewee selected)
    TEST_ASSERT_EQUAL_STRING("Add Player", game.textDisplay.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("Geewee", game.textDisplay.captured_item.c_str());

    // 3. Navigate to "Coach"
    simulateRotation(game, 1); // Sammy
    simulateRotation(game, 1); // Coach

    // 4. Add Coach
    simulateButtonPress(game, ButtonAction::SELECT);
    TEST_ASSERT_EQUAL_INT(1, game.state.players.size());
    TEST_ASSERT_EQUAL_STRING("Coach", game.state.players[0].name.c_str());

    // 5. Add "Alex"
    // After adding Coach (index 2), Sheshe is now at index 2.
    // Alex is at index 3. So one Rotation is needed.
    simulateRotation(game, 1); // Alex
    simulateButtonPress(game, ButtonAction::SELECT);
    TEST_ASSERT_EQUAL_INT(2, game.state.players.size());
    TEST_ASSERT_EQUAL_STRING("Alex", game.state.players[1].name.c_str());

    // 6. Start Game
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Should be in WaitingPhase, showing first player (Coach)
    TEST_ASSERT_EQUAL_STRING("Coach", game.textDisplay.captured_message.c_str());
    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);
}

// Verifies that a standard turn correctly banks the score and advances to the next player.
void test_TurnLifecycle_StandardTurn() {
    Game game;
    setupGameWithPlayers(game, 4);

    // Simulate scoring 1500 (3 x 500)
    simulateButtonPress(game, ButtonAction::PLUS_500, 3);
    
    // Start banking
    simulateButtonPress(game, ButtonAction::BANK);

    while(game.state.atRiskScore > 0) {
        simulateNoAction(game);
    }

    simulateButtonPress(game, ButtonAction::BANK);

    TEST_ASSERT_EQUAL_INT(1, game.state.currentPlayerIndex);
    TEST_ASSERT_EQUAL_INT(1500, game.state.players[0].score);
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
}

// Verifies that the game correctly cycles through all players.
void test_TurnLifecycle_RoundRobin() {
    Game game;
    setupGameWithPlayers(game, 4);

    for (int i = 0; i < 4; i++) {
        simulateButtonPress(game, ButtonAction::PLUS_500, 2);
        
        // Start banking
        simulateButtonPress(game, ButtonAction::BANK);

        while(game.state.atRiskScore > 0) {
            simulateNoAction(game);
        }

        simulateButtonPress(game, ButtonAction::BANK);
    }

    TEST_ASSERT_EQUAL_INT(0, game.state.currentPlayerIndex);
}

// Verifies that the clear button resets the atRiskScore to 0.
void test_TurnLifecycle_ClearButton() {
    Game game;
    setupGameWithPlayers(game, 4);
    simulateButtonPress(game, ButtonAction::PLUS_500, 2);
    simulateButtonPress(game, ButtonAction::CLEAR);

    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
}

void run_turn_lifecycle_tests() {
    RUN_TEST(test_TurnLifecycle_FullSetupAndTurn);
    RUN_TEST(test_TurnLifecycle_StandardTurn);
    RUN_TEST(test_TurnLifecycle_RoundRobin);
    RUN_TEST(test_TurnLifecycle_ClearButton);
}
