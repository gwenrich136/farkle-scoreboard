#include "test_PlayerSelectionPhase.h"
#include "Game.h"
#include "phases/PlayerSelectionPhase.h"
#include "phases/WaitingPhase.h"
#include "../test_utils.h"
#include <unity.h>

void test_PlayerSelectionPhase_Navigation() {
    Game game;
    game.setup();

    // Initial state should be PlayerSelectionPhase
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PlayerSelectionPhase>(), game.currentPhase);

    // Test navigation doesn't crash and stays in phase
    simulateButtonPress(game, ButtonAction::UP_1000);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PlayerSelectionPhase>(), game.currentPhase);

    simulateButtonPress(game, ButtonAction::DOWN_50);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PlayerSelectionPhase>(), game.currentPhase);
}

void test_PlayerSelectionPhase_AddPlayer() {
    Game game;
    game.setup();

    TEST_ASSERT_EQUAL_INT(0, game.state.players.size());

    // Press BANK to add Geewee
    simulateButtonPress(game, ButtonAction::BANK);

    TEST_ASSERT_EQUAL_INT(1, game.state.players.size());
    TEST_ASSERT_EQUAL_STRING("Geewee", game.state.players[0].name.c_str());
}

void test_PlayerSelectionPhase_FilterDuplicateNames() {
    Game game;
    game.setup();

    // Add Geewee
    simulateButtonPress(game, ButtonAction::BANK);
    TEST_ASSERT_EQUAL_INT(1, game.state.players.size());

    // Press BANK again. Navigation should have moved to next name (Sammy) because Geewee is filtered
    simulateButtonPress(game, ButtonAction::BANK);
    TEST_ASSERT_EQUAL_INT(2, game.state.players.size());
    TEST_ASSERT_EQUAL_STRING("Sammy", game.state.players[1].name.c_str());
}

void test_PlayerSelectionPhase_MaxPlayersLimit() {
    Game game;
    game.setup();

    // Add 8 players (max)
    for (int i = 0; i < 8; ++i) {
        simulateButtonPress(game, ButtonAction::BANK);
    }
    TEST_ASSERT_EQUAL_INT(8, game.state.players.size());

    // Try to add 9th
    simulateButtonPress(game, ButtonAction::BANK);
    TEST_ASSERT_EQUAL_INT(8, game.state.players.size());
}

void test_PlayerSelectionPhase_TransitionToWaiting() {
    Game game;
    game.setup();

    // Try to start with 0 players
    simulateButtonPress(game, ButtonAction::FARKLE);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<PlayerSelectionPhase>(), game.currentPhase);

    // Add 1 player
    simulateButtonPress(game, ButtonAction::BANK);

    // Try to start with 1 player
    simulateButtonPress(game, ButtonAction::FARKLE);
    TEST_ASSERT_EQUAL_PTR(game.getPhase<WaitingPhase>(), game.currentPhase);
}

void run_player_selection_phase_tests() {
    RUN_TEST(test_PlayerSelectionPhase_Navigation);
    RUN_TEST(test_PlayerSelectionPhase_AddPlayer);
    RUN_TEST(test_PlayerSelectionPhase_FilterDuplicateNames);
    RUN_TEST(test_PlayerSelectionPhase_MaxPlayersLimit);
    RUN_TEST(test_PlayerSelectionPhase_TransitionToWaiting);
}
