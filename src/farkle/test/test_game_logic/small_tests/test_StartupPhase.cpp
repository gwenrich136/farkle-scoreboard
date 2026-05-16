#include "test_StartupPhase.h"
#include "Game.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that the phase starts correctly and displays the correct title and item.
void test_StartupPhase_InitialState() {
    Game game;
    game.getMemoryCard().mock_hasActiveGame_result = false;
    game.setup();
    game.loop();

    // Should start in StartupPhase
    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("New Game", game.textDisplay.captured_item.c_str());
}

// Verifies that pressing SELECT transitions to TargetScoreSelectionPhase.
void test_StartupPhase_Transition() {
    Game game;
    game.getMemoryCard().mock_hasActiveGame_result = false;
    game.setup();

    // Transition with SELECT
    simulateButtonPress(game, ButtonAction::SELECT);

    // Should be in TargetScoreSelectionPhase
    TEST_ASSERT_EQUAL_STRING("Target Score", game.textDisplay.captured_title.c_str());
}

// Verifies that rotating does nothing.
void test_StartupPhase_IgnoreRotation() {
    Game game;
    game.getMemoryCard().mock_hasActiveGame_result = false;
    game.setup();

    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());

    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());
}

// Verifies resume active game initial state
void test_StartupPhase_Resume_InitialState() {
    Game game;
    game.getMemoryCard().mock_hasActiveGame_result = true;
    game.setup();
    game.loop();

    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("Resume Game", game.textDisplay.captured_item.c_str());
}

// Verifies navigating the resume menu
void test_StartupPhase_Resume_Navigation() {
    Game game;
    game.getMemoryCard().mock_hasActiveGame_result = true;
    game.setup();

    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("New Game", game.textDisplay.captured_item.c_str());

    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_STRING("Resume Game", game.textDisplay.captured_item.c_str());
}

// Verifies resuming an active game correctly transitions and loads
void test_StartupPhase_Resume_Transition() {
    Game game;
    game.getMemoryCard().mock_hasActiveGame_result = true;
    game.getMemoryCard().mock_loadGameMetadata_result = true;
    game.getMemoryCard().mock_replayGameJournal_result = true;

    game.setup();
    // Simulate what loadGameMetadata would do
    game.state.players.push_back(Player("Alice"));

    // Press SELECT. simulateButtonPress calls game.loop() implicitly,
    // which processes the SELECT, transitions to WaitingPhase, and calls its onEnter
    simulateButtonPress(game, ButtonAction::SELECT);

    // Should be in WaitingPhase
    // Check head-to-head title properties from InGamePhase
    TEST_ASSERT_EQUAL_STRING("Alice", game.textDisplay.captured_p1Name.c_str());
    TEST_ASSERT_TRUE(game.getMemoryCard().mock_loadGameMetadata_called);
    TEST_ASSERT_TRUE(game.getMemoryCard().mock_replayGameJournal_called);
}

// Verifies selecting "New Game" when resume was an option
void test_StartupPhase_Resume_NewGameSelection() {
    Game game;
    game.getMemoryCard().mock_hasActiveGame_result = true;
    game.setup();

    simulateRotation(game, 1); // switch to New Game
    simulateButtonPress(game, ButtonAction::SELECT);

    TEST_ASSERT_EQUAL_STRING("Target Score", game.textDisplay.captured_title.c_str());
    TEST_ASSERT_FALSE(game.getMemoryCard().mock_clearActiveGame_called); // Should NOT clear active game
}

// Verifies falling back to StartupPhase if resume fails
void test_StartupPhase_Resume_FallbackOnFailure() {
    Game game;
    game.getMemoryCard().mock_hasActiveGame_result = true;
    game.getMemoryCard().mock_loadGameMetadata_result = false; // fail!
    game.setup();

    // After failure, it should clear the broken active game and return to StartupPhase.
    // However, since mock_hasActiveGame_result is hardcoded to true in the mock, populateOptions
    // inside launchResumeGame will still think there is an active game. Let's toggle the mock result
    // to simulate clearing it properly.
    game.getMemoryCard().mock_hasActiveGame_result = false;

    simulateButtonPress(game, ButtonAction::SELECT);

    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("New Game", game.textDisplay.captured_item.c_str());
    TEST_ASSERT_TRUE(game.getMemoryCard().mock_clearActiveGame_called);
}

// Verifies that other buttons do nothing.
void test_StartupPhase_IgnoreOtherButtons() {
    Game game;
    game.setup();

    simulateButtonPress(game, ButtonAction::FARKLE);
    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());

    simulateButtonPress(game, ButtonAction::BANK);
    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());
}

void run_startup_phase_tests() {
    RUN_TEST(test_StartupPhase_InitialState);
    RUN_TEST(test_StartupPhase_Transition);
    RUN_TEST(test_StartupPhase_IgnoreRotation);
    RUN_TEST(test_StartupPhase_IgnoreOtherButtons);
    RUN_TEST(test_StartupPhase_Resume_InitialState);
    RUN_TEST(test_StartupPhase_Resume_Navigation);
    RUN_TEST(test_StartupPhase_Resume_Transition);
    RUN_TEST(test_StartupPhase_Resume_NewGameSelection);
    RUN_TEST(test_StartupPhase_Resume_FallbackOnFailure);
}
