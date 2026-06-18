#include "Game.h"

#ifndef TEST_FULL_GAME_H
#define TEST_FULL_GAME_H

void test_FullGame_StandardGame();
void test_FullGame_TripleFarkle();
void test_FullGame_TripleFarkle_ScoreLessThanPenalty();
void test_FullGame_FinalRoundBlinking();
void test_PostGame_FinalizeCalledOnce();
void test_FullGame_ResumeActiveGame_GridInitialization();
void test_FullGame_AllGameSoundsTriggered();
void test_FullGame_SystemSoundsTriggered();
void run_full_game_tests();
void advance_to_player_zero(Game& game);

#endif // TEST_FULL_GAME_H
