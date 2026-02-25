#include "test_multi_press.h"
#include "../test_utils.h"
#include <unity.h>

void test_simulate_button_press_count() {
    Game game;
    setupGameWithPlayers(game, 2); // 2 players

    // Player 1 is active.
    // Simulate pressing PLUS_500 twice.
    // Expected: Current roll score increases by 1000.

    // This call uses the new feature: count = 2.
    // If implementation is not updated, it treats '2' as 'time', so only 1 press happens -> 500 points.
    // If implementation is updated, it presses twice -> 1000 points.
    simulateButtonPress(game, ButtonAction::PLUS_500, 2);

    TEST_ASSERT_EQUAL_INT(1000, game.state.atRiskScore);
}

void run_multi_press_tests() {
    RUN_TEST(test_simulate_button_press_count);
}
