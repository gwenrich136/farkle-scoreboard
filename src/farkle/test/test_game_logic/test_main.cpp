#include <unity.h>
#include "test_WaitingPhase.h"
#include "test_FarklingPhase.h"
#include "test_PenaltyFarklingPhase.h"
#include "test_BankingPhase.h"
#include "test_full_game.h"
#include "test_turn_lifecycle.h"

void setUp(void) {
    // set up tear down functions that are required by unity
}

void tearDown(void) {
    // set up tear down functions that are required by unity
}

void test_runner() {
    run_waiting_phase_tests();
    run_farkling_phase_tests();
    run_penalty_farkling_phase_tests();
    run_banking_phase_tests();
    run_full_game_tests();
    run_turn_lifecycle_tests();
}

int main() {
    UNITY_BEGIN();
    test_runner();
    return UNITY_END();
}
