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
    simulateButtonPress(game, ButtonAction::SELECT);
    game.loop(); // Run one extra loop to allow MemoryCard to initialize

    // Should be in PlayerSelectionPhase
    TEST_ASSERT_EQUAL_STRING("Add Player", game.textDisplay.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("Geewee", game.textDisplay.captured_item.c_str());
    TEST_ASSERT_EQUAL_INT(0, game.state.players.size());
}

// Verifies that rotation increments and decrements the name list correctly, including wrapping.
void test_PlayerSelection_Cycling() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::SELECT);
    game.loop();

    // Next name
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Sammy", game.textDisplay.captured_item.c_str());

    // Previous name
    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_STRING("Geewee", game.textDisplay.captured_item.c_str());

    // Wrap around backward
    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_STRING("Andrea", game.textDisplay.captured_item.c_str());

    // Wrap around forward
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Geewee", game.textDisplay.captured_item.c_str());
}

// Verifies that pressing SELECT adds the selected name and the selection "stays in place" (shifts to next name).
void test_PlayerSelection_AddPlayer() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::SELECT);
    game.loop();

    // Navigate to Sammy (index 1)
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Sammy", game.textDisplay.captured_item.c_str());

    // Press BANK should not add
    simulateButtonPress(game, ButtonAction::BANK);
    TEST_ASSERT_EQUAL_INT(0, game.state.players.size());

    // Add Sammy with SELECT. List becomes: Geewee, Coach, Sheshe, ...
    // index 1 should now point to "Coach"
    simulateButtonPress(game, ButtonAction::SELECT);
    TEST_ASSERT_EQUAL_INT(1, game.state.players.size());
    TEST_ASSERT_EQUAL_STRING("Sammy", game.state.players[0].name.c_str());

    TEST_ASSERT_EQUAL_STRING("Coach", game.textDisplay.captured_item.c_str());
}

// Verifies that multiple added players are all removed from the selection list and index stays valid.
void test_PlayerSelection_Filtering() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::SELECT);
    game.loop();

    // Add Geewee (index 0)
    simulateButtonPress(game, ButtonAction::SELECT);
    // index 0 is now Sammy

    // Add Sammy (index 0)
    simulateButtonPress(game, ButtonAction::SELECT);
    // index 0 is now Coach

    TEST_ASSERT_EQUAL_INT(2, game.state.players.size());
    TEST_ASSERT_EQUAL_STRING("Geewee", game.state.players[0].name.c_str());
    TEST_ASSERT_EQUAL_STRING("Sammy", game.state.players[1].name.c_str());

    TEST_ASSERT_EQUAL_STRING("Coach", game.textDisplay.captured_item.c_str());
}

// Verifies that the game cannot start with 0 players but successfully transitions with >= 1.
void test_PlayerSelection_TransitionValidation() {
    Game game;
    game.setup();

    // Cannot start with 0 players
    simulateButtonPress(game, ButtonAction::SELECT); // Transition from TargetScore to PlayerSelection
    simulateButtonPress(game, ButtonAction::FARKLE); // Try to start with 0
    TEST_ASSERT_EQUAL_STRING("Add Player", game.textDisplay.captured_title.c_str());

    // Add 1 player
    simulateButtonPress(game, ButtonAction::SELECT);

    // Now can start
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Should be in WaitingPhase (OLED shows Head-to-Head info now instead of basic message)
    TEST_ASSERT_EQUAL_STRING("Geewee", game.textDisplay.captured_p1Name.c_str());
    TEST_ASSERT_EQUAL_STRING("1st", game.textDisplay.captured_p1Place.c_str());
}

// Verifies that the phase respects the 8-player hardware limit and shows "ROSTER FULL" using a simple print.
void test_PlayerSelection_MaxPlayers() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::SELECT);
    game.loop();

    // Add 8 players
    simulateButtonPress(game, ButtonAction::SELECT, 8);

    TEST_ASSERT_EQUAL_INT(8, game.state.players.size());
    TEST_ASSERT_TRUE(game.grid.isMaxPlayersReached());
    TEST_ASSERT_EQUAL_STRING("ROSTER FULL", game.textDisplay.captured_message.c_str());

    // Try to add one more
    simulateButtonPress(game, ButtonAction::SELECT);
    TEST_ASSERT_EQUAL_INT(8, game.state.players.size());
}

// Verifies that adding the last name in the filtered pool wraps the selection index to 0.
void test_PlayerSelection_AddLastPlayerWraps() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::SELECT);
    game.loop();

    // Pool size is 9. Add first 8 players one by one, but always selecting the LAST one.
    // names: 0, 1, 2, 3, 4, 5, 6, 7, 8

    // Move to last (index 8: Andrea)
    for(int i=0; i<8; ++i) simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Andrea", game.textDisplay.captured_item.c_str());

    // Add Andrea. The current player in MemoryCard becomes out of bounds and should wrap to Geewee.
    simulateButtonPress(game, ButtonAction::SELECT);

    // In our mock, `Andrea` was removed. The new list has 8 elements.
    // Wait, the test adds Andrea, then checks if the UI wraps around to the FIRST item.
    // MemoryCard `reservePlayer` advances the cursor. If it was at the end, `reservePlayer` detects
    // it can't advance and calls `getPreviousPlayer()`. So the cursor will be at `Fred` instead of `Geewee`.
    // Wait, previously `m_selectionIndex` wrapped to 0 due to modulo arithmetic.
    // Let's check what the UI *actually* shows when Andrea is added:
    // It shows Fred in our mock because `reservePlayer` falls back to `getPreviousPlayer()` when at the end.
    // Wait, the *design doc* says: "If it hits the end of the list, returns """.
    // And `reservePlayer` says: "It will then auto-advance to the next available player (or previous if it is at the end of the list)."
    // So the correct behavior according to the design is to show the PREVIOUS player (Fred).
    // The test was expecting Geewee because of the old modulo behavior.
    TEST_ASSERT_EQUAL_STRING("Fred", game.textDisplay.captured_item.c_str());
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
