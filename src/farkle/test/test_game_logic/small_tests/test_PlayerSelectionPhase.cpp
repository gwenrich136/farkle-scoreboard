#include "test_PlayerSelectionPhase.h"
#include "Game.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

void test_PlayerSelection_InitialState() {
    Game game;
    game.setup();
    game.loop();

    // Should start in PlayerSelectionPhase
    TEST_ASSERT_EQUAL_STRING("Add Player", game.oled.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("Geewee", game.oled.captured_item.c_str());
    TEST_ASSERT_EQUAL_INT(0, game.state.players.size());
}

void test_PlayerSelection_Navigation() {
    Game game;
    game.setup();

    // Next name
    simulateButtonPress(game, ButtonAction::UP_1000);
    TEST_ASSERT_EQUAL_STRING("Sammy", game.oled.captured_item.c_str());

    // Previous name
    simulateButtonPress(game, ButtonAction::DOWN_50);
    TEST_ASSERT_EQUAL_STRING("Geewee", game.oled.captured_item.c_str());

    // Wrap around backward
    simulateButtonPress(game, ButtonAction::DOWN_50);
    TEST_ASSERT_EQUAL_STRING("Andrea", game.oled.captured_item.c_str());

    // Wrap around forward
    simulateButtonPress(game, ButtonAction::UP_1000);
    TEST_ASSERT_EQUAL_STRING("Geewee", game.oled.captured_item.c_str());
}

void test_PlayerSelection_AddPlayer() {
    Game game;
    game.setup();

    // Add Geewee
    simulateButtonPress(game, ButtonAction::BANK);
    TEST_ASSERT_EQUAL_INT(1, game.state.players.size());
    TEST_ASSERT_EQUAL_STRING("Geewee", game.state.players[0].name.c_str());

    // Geewee should be filtered out, next should be Sammy
    TEST_ASSERT_EQUAL_STRING("Sammy", game.oled.captured_item.c_str());
}

void test_PlayerSelection_Filtering() {
    Game game;
    game.setup();

    // Add Geewee
    simulateButtonPress(game, ButtonAction::BANK);

    // Add Sammy
    simulateButtonPress(game, ButtonAction::BANK);

    TEST_ASSERT_EQUAL_INT(2, game.state.players.size());
    TEST_ASSERT_EQUAL_STRING("Geewee", game.state.players[0].name.c_str());
    TEST_ASSERT_EQUAL_STRING("Sammy", game.state.players[1].name.c_str());

    // Next available should be Coach
    TEST_ASSERT_EQUAL_STRING("Coach", game.oled.captured_item.c_str());
}

void test_PlayerSelection_TransitionToWaiting() {
    Game game;
    game.setup();

    // Cannot start with 0 players
    simulateButtonPress(game, ButtonAction::FARKLE);
    TEST_ASSERT_EQUAL_STRING("Add Player", game.oled.captured_title.c_str());

    // Add 1 player
    simulateButtonPress(game, ButtonAction::BANK);

    // Now can start
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Should be in WaitingPhase (OLED shows player name)
    TEST_ASSERT_EQUAL_STRING("Geewee", game.oled.captured_message.c_str());
}

void test_PlayerSelection_MaxPlayers() {
    Game game;
    game.setup();

    // Add 8 players
    for (int i = 0; i < 8; ++i) {
        simulateButtonPress(game, ButtonAction::BANK);
    }

    TEST_ASSERT_EQUAL_INT(8, game.state.players.size());
    TEST_ASSERT_TRUE(game.grid.isMaxPlayersReached());
    TEST_ASSERT_EQUAL_STRING("ROSTER FULL", game.oled.captured_item.c_str());

    // Try to add one more
    simulateButtonPress(game, ButtonAction::BANK);
    TEST_ASSERT_EQUAL_INT(8, game.state.players.size());
}

void run_player_selection_phase_tests() {
    RUN_TEST(test_PlayerSelection_InitialState);
    RUN_TEST(test_PlayerSelection_Navigation);
    RUN_TEST(test_PlayerSelection_AddPlayer);
    RUN_TEST(test_PlayerSelection_Filtering);
    RUN_TEST(test_PlayerSelection_TransitionToWaiting);
    RUN_TEST(test_PlayerSelection_MaxPlayers);
}
