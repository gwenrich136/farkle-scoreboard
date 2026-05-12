#include "test_StartupPhase.h"
#include "Game.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that the phase starts correctly and displays the correct title and item.
void test_StartupPhase_InitialState() {
    Game game;
    game.setup();
    game.loop();

    // Should start in StartupPhase
    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("New Game", game.textDisplay.captured_item.c_str());
}

// Verifies that pressing SELECT transitions to TargetScoreSelectionPhase.
void test_StartupPhase_Transition() {
    Game game;
    game.setup();

    // Transition with SELECT
    simulateButtonPress(game, ButtonAction::SELECT);

    // Should be in TargetScoreSelectionPhase
    TEST_ASSERT_EQUAL_STRING("Target Score", game.textDisplay.captured_title.c_str());
}

// Verifies that rotating does nothing.
void test_StartupPhase_IgnoreRotation() {
    Game game;
    game.setup();

    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());

    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_STRING("Farkle!", game.textDisplay.captured_title.c_str());
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
}
