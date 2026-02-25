#include "test_PlayerSelectionPhase.h"
#include "Game.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that the phase starts with the first name in the pool ("Geewee") and an empty player list.
void test_PlayerSelection_InitialState() {
    Game game;
    game.setup();

    // Transition to PlayerSelectionPhase
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Should be in PlayerSelectionPhase
    TEST_ASSERT_EQUAL_STRING("Add Player", game.oled.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("Geewee", game.oled.captured_item.c_str());
    TEST_ASSERT_EQUAL_INT(0, game.state.players.size());
}

// Verifies that rotation increments and decrements the name list correctly, including wrapping.
void test_PlayerSelection_Cycling() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Next name
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Sammy", game.oled.captured_item.c_str());

    // Previous name
    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_STRING("Geewee", game.oled.captured_item.c_str());

    // Wrap around backward
    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_STRING("Andrea", game.oled.captured_item.c_str());

    // Wrap around forward
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Geewee", game.oled.captured_item.c_str());
}

// Verifies that pressing BANK adds the selected name and the selection "stays in place" (shifts to next name).
void test_PlayerSelection_AddPlayer() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Navigate to Sammy (index 1)
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Sammy", game.oled.captured_item.c_str());

    // Add Sammy. List becomes: Geewee, Coach, Sheshe, ...
    // index 1 should now point to "Coach"
    simulateButtonPress(game, ButtonAction::BANK);
    TEST_ASSERT_EQUAL_INT(1, game.state.players.size());
    TEST_ASSERT_EQUAL_STRING("Sammy", game.state.players[0].name.c_str());

    TEST_ASSERT_EQUAL_STRING("Coach", game.oled.captured_item.c_str());
}

// Verifies that multiple added players are all removed from the selection list and index stays valid.
void test_PlayerSelection_Filtering() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Add Geewee (index 0)
    simulateButtonPress(game, ButtonAction::BANK);
    // index 0 is now Sammy

    // Add Sammy (index 0)
    simulateButtonPress(game, ButtonAction::BANK);
    // index 0 is now Coach

    TEST_ASSERT_EQUAL_INT(2, game.state.players.size());
    TEST_ASSERT_EQUAL_STRING("Geewee", game.state.players[0].name.c_str());
    TEST_ASSERT_EQUAL_STRING("Sammy", game.state.players[1].name.c_str());

    TEST_ASSERT_EQUAL_STRING("Coach", game.oled.captured_item.c_str());
}

// Verifies that the game cannot start with 0 players but successfully transitions with >= 1.
void test_PlayerSelection_TransitionValidation() {
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

// Verifies that the phase respects the 8-player hardware limit and shows "ROSTER FULL" using a simple print.
void test_PlayerSelection_MaxPlayers() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Add 8 players
    for (int i = 0; i < 8; ++i) {
        simulateButtonPress(game, ButtonAction::BANK);
    }

    TEST_ASSERT_EQUAL_INT(8, game.state.players.size());
    TEST_ASSERT_TRUE(game.grid.isMaxPlayersReached());
    TEST_ASSERT_EQUAL_STRING("ROSTER FULL", game.oled.captured_message.c_str());

    // Try to add one more
    simulateButtonPress(game, ButtonAction::BANK);
    TEST_ASSERT_EQUAL_INT(8, game.state.players.size());
}

// Verifies that adding the last name in the filtered pool wraps the selection index to 0.
void test_PlayerSelection_AddLastPlayerWraps() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Pool size is 9. Add first 8 players one by one, but always selecting the LAST one.
    // names: 0, 1, 2, 3, 4, 5, 6, 7, 8

    // Move to last (index 8: Andrea)
    for(int i=0; i<8; ++i) simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Andrea", game.oled.captured_item.c_str());

    // Add Andrea. List size becomes 8. Index 8 is out of bounds. Should wrap to 0 (Geewee).
    simulateButtonPress(game, ButtonAction::BANK);
    TEST_ASSERT_EQUAL_STRING("Geewee", game.oled.captured_item.c_str());
}

void run_player_selection_phase_tests() {
    RUN_TEST(test_PlayerSelection_InitialState);
    RUN_TEST(test_PlayerSelection_Cycling);
    RUN_TEST(test_PlayerSelection_AddPlayer);
    RUN_TEST(test_PlayerSelection_Filtering);
    RUN_TEST(test_PlayerSelection_TransitionValidation);
    RUN_TEST(test_PlayerSelection_MaxPlayers);
    RUN_TEST(test_PlayerSelection_AddLastPlayerWraps);
}
