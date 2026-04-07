#include <unity.h>
#include "small_tests/test_TargetScoreSelectionPhase.h"
#include "small_tests/test_PlayerSelectionPhase.h"
#include "small_tests/test_WaitingPhase.h"
#include "small_tests/test_FarklingPhase.h"
#include "small_tests/test_PenaltyFarklingPhase.h"
#include "small_tests/test_BankingPhase.h"
#include "small_tests/test_EndOfTurnPhase.h"
#include "large_tests/test_full_game.h"
#include "medium_tests/test_turn_lifecycle.h"
#include "medium_tests/test_conditional_at_risk_display.h"
#include "medium_tests/test_tie_breaking.h"
#include "small_tests/test_multi_press.h"

void setUp(void) {
    // set up tear down functions that are required by unity
}

void tearDown(void) {
    // set up tear down functions that are required by unity
}

void test_runner() {
    run_target_score_selection_phase_tests();
    run_player_selection_phase_tests();
    run_waiting_phase_tests();
    run_farkling_phase_tests();
    run_penalty_farkling_phase_tests();
    run_banking_phase_tests();
    run_end_of_turn_phase_tests();
    run_full_game_tests();
    run_turn_lifecycle_tests();
    run_display_logic_tests();
    run_tie_breaking_tests();
    run_multi_press_tests();
}

int main() {
    UNITY_BEGIN();
    test_runner();
    return UNITY_END();
}
