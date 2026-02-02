#include "test_FarklingPhase.h"
#include <unity.h>

// Verifies that a player's score is penalized by 1000 points after three consecutive farkles.
void test_FarklingPhase_TripleFarklePenalty() {
    // TODO: Set player's farkle_count to 2
    // TODO: Trigger transition to FarklingPhase
    // TODO: Assert player's score decreases by 1000
}

void run_farkling_phase_tests() {
    RUN_TEST(test_FarklingPhase_TripleFarklePenalty);
}
