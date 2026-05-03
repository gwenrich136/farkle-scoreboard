#include <unity.h>
#include "Game.h"
#include "test_Security.h"
#include "Arduino.h"

void test_random_seed_initialized_on_setup() {
    lastRandomSeed = 0;
    setMockAnalogPin(A0, 123);
    setMockAnalogPin(A1, 456);
    setMockAnalogPin(A2, 789);
    advance_millis(1000);

    Game game;
    game.setup();

    TEST_ASSERT_NOT_EQUAL(0, lastRandomSeed);
}

void run_security_tests() {
    RUN_TEST(test_random_seed_initialized_on_setup);
}
