#include <unity.h>

// Forward declarations for test suites
void run_transition_tests();
void run_scoring_tests();

void setUp(void) {
    // Set up things before each test
}

void tearDown(void) {
    // Clean up things after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    run_transition_tests();
    run_scoring_tests();
    return UNITY_END();
}
