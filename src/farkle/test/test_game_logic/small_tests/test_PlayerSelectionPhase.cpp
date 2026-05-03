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

// Verifies that rotation increments and decrements the name list correctly, including boundaries.
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

    // Hit top boundary
    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_STRING("NEW PLAYER", game.textDisplay.captured_item.c_str());

    // Dead action: hit top boundary again
    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_STRING("NEW PLAYER", game.textDisplay.captured_item.c_str());

    // Go forward back to Geewee
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

    // Verify MemoryCard file lifecycle methods were called
    TEST_ASSERT_TRUE(game.getMemoryCard().mock_getOrGenerateNextGameId_called);
    TEST_ASSERT_TRUE(game.getMemoryCard().mock_setActiveGameId_called);
    TEST_ASSERT_EQUAL_UINT32(42, game.getMemoryCard().mock_setActiveGameId_arg);
    TEST_ASSERT_TRUE(game.getMemoryCard().mock_initializeGameDirectory_called);
    TEST_ASSERT_EQUAL_UINT32(42, game.getMemoryCard().mock_initializeGameDirectory_arg);
    TEST_ASSERT_TRUE(game.getMemoryCard().mock_writeGameMetadata_called);
    TEST_ASSERT_EQUAL_UINT32(42, game.getMemoryCard().mock_writeGameMetadata_arg);

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

// Verifies that adding the last name in the filtered pool falls back to the previous name, not wrapping to 0.
void test_PlayerSelection_AddLastPlayerFallsBack() {
    Game game;
    game.setup();
    simulateButtonPress(game, ButtonAction::SELECT);
    game.loop();

    // Pool size is 9. Move to last (index 8: Andrea)
    for(int i=0; i<8; ++i) simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Andrea", game.textDisplay.captured_item.c_str());

    // Add Andrea. The current player in MemoryCard becomes out of bounds.
    // reservePlayer() will fall back to getPreviousPlayer(), so it should go to Fred.
    simulateButtonPress(game, ButtonAction::SELECT);

    TEST_ASSERT_EQUAL_STRING("Fred", game.textDisplay.captured_item.c_str());
}

void run_player_selection_phase_tests() {
    RUN_TEST(test_PlayerSelection_InitialState);
    RUN_TEST(test_PlayerSelection_Cycling);
    RUN_TEST(test_PlayerSelection_AddPlayer);
    RUN_TEST(test_PlayerSelection_Filtering);
    RUN_TEST(test_PlayerSelection_TransitionValidation);
    RUN_TEST(test_PlayerSelection_MaxPlayers);
    RUN_TEST(test_PlayerSelection_AddLastPlayerFallsBack);
}
