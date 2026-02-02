#include <unity.h>
#include "Game.h"

// Test function to verify score accumulation
void test_score_accumulation() {
    Game game;
    game.setup();

    // Ensure atRiskScore is initially 0
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);

    // Simulate pressing UP_1000 and RIGHT_500
    game.controlPad.press(ButtonAction::UP_1000);
    game.loop();
    game.controlPad.press(ButtonAction::RIGHT_500);
    game.loop();

    // Verify atRiskScore is 1500
    TEST_ASSERT_EQUAL_INT(1500, game.state.atRiskScore);
}

// Test function to verify score clearing
void test_score_clearing() {
    Game game;
    game.setup();

    // Add some score
    game.controlPad.press(ButtonAction::UP_1000);
    game.loop();
    TEST_ASSERT_EQUAL_INT(1000, game.state.atRiskScore);

    // Simulate pressing CLEAR
    game.controlPad.press(ButtonAction::CLEAR);
    game.loop();

    // Verify atRiskScore is 0
    TEST_ASSERT_EQUAL_INT(0, game.state.atRiskScore);
}

void run_scoring_tests() {
    RUN_TEST(test_score_accumulation);
    RUN_TEST(test_score_clearing);
}
