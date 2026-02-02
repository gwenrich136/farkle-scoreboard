#include <unity.h>
#include "test_WaitingPhase.h"
#include "test_FarklingPhase.h"

void setUp(void) {
    // set up tear down functions that are required by unity
}

void tearDown(void) {
    // set up tear down functions that are required by unity
}

void test_runner() {
    RUN_TEST(test_WaitingPhase_ScoreAccumulation);
    RUN_TEST(test_WaitingPhase_ScoreCorrection);
    RUN_TEST(test_WaitingPhase_TransitionToBanking);
    RUN_TEST(test_WaitingPhase_TransitionToFarkling);
    RUN_TEST(test_FarklingPhase_TripleFarklePenalty);
}

int main() {
    UNITY_BEGIN();
    test_runner();
    return UNITY_END();
}
