#include "test_TargetScoreSelectionPhase.h"
#include "Game.h"
#include "../test_utils.h"
#include <unity.h>
#include "Arduino.h"

// Verifies that the phase starts with the default target score (10,000).
void test_TargetScoreSelection_InitialState() {
    Game game;
    game.setup();
    game.loop();

    // Should start in TargetScoreSelectionPhase
    TEST_ASSERT_EQUAL_STRING("Target Score", game.oled.captured_title.c_str());
    TEST_ASSERT_EQUAL_STRING("10,000", game.oled.captured_item.c_str());
    TEST_ASSERT_EQUAL_INT(10000, game.state.targetScore);
}

// Verifies that rotation increments and decrements the target score correctly.
void test_TargetScoreSelection_Adjustment() {
    Game game;
    game.setup();

    // Increment (1 click = 1000)
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_INT(11000, game.state.targetScore);
    TEST_ASSERT_EQUAL_STRING("11,000", game.oled.captured_item.c_str());

    // Decrement (1 click = 1000)
    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_INT(10000, game.state.targetScore);
    TEST_ASSERT_EQUAL_STRING("10,000", game.oled.captured_item.c_str());
}

// Verifies that the target score is clamped between 1,000 and 20,000.
void test_TargetScoreSelection_Clamping() {
    Game game;
    game.setup();

    // Test Lower Bound
    // Start at 10,000. Go down 15,000 (15 clicks).
    // Wait, 15 clicks * 1000 = 15000. 10000 - 15000 = -5000 -> clamped to 1000?
    // Let's do 9 clicks to get to 1000.
    for (int i = 0; i < 9; ++i) {
        simulateRotation(game, -1);
    }
    TEST_ASSERT_EQUAL_INT(1000, game.state.targetScore);
    TEST_ASSERT_EQUAL_STRING("1,000", game.oled.captured_item.c_str());

    // Try to go below
    simulateRotation(game, -1);
    TEST_ASSERT_EQUAL_INT(1000, game.state.targetScore);

    // Test Upper Bound
    // Start at 1000. Go up 19 clicks -> 20000.
    for (int i = 0; i < 25; ++i) {
        simulateRotation(game, 1);
    }
    // Should be clamped at 20000
    TEST_ASSERT_EQUAL_INT(20000, game.state.targetScore);
    TEST_ASSERT_EQUAL_STRING("20,000", game.oled.captured_item.c_str());

    // Try to go above
    simulateRotation(game, 1);
    TEST_ASSERT_EQUAL_INT(20000, game.state.targetScore);
}

// Verifies that pressing BANK or FARKLE transitions to PlayerSelectionPhase.
void test_TargetScoreSelection_Transition() {
    Game game;
    game.setup();

    // Change score to 5000
    for (int i = 0; i < 5; ++i) {
        simulateRotation(game, -1);
    }
    TEST_ASSERT_EQUAL_INT(5000, game.state.targetScore);

    // Transition with BANK
    simulateButtonPress(game, ButtonAction::BANK);

    // Should be in PlayerSelectionPhase
    TEST_ASSERT_EQUAL_STRING("Add Player", game.oled.captured_title.c_str());

    // Reset and test transition with FARKLE
    game.setup();
    simulateButtonPress(game, ButtonAction::FARKLE);
    TEST_ASSERT_EQUAL_STRING("Add Player", game.oled.captured_title.c_str());
}

void run_target_score_selection_phase_tests() {
    RUN_TEST(test_TargetScoreSelection_InitialState);
    RUN_TEST(test_TargetScoreSelection_Adjustment);
    RUN_TEST(test_TargetScoreSelection_Clamping);
    RUN_TEST(test_TargetScoreSelection_Transition);
}
