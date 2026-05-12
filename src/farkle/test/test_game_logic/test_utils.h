#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "Game.h"
#include "Input.h"

void simulateButtonPress(Game& game, ButtonAction action, int count = 1, unsigned long advance_time_millis = 10);
void simulateRotation(Game& game, int delta, unsigned long advance_time_millis = 10);
// Removed simulateScore
void simulateNoAction(Game& game, unsigned long advance_time_millis = 10);
void waitForScoreAnimation(Game& game);
void setupGameWithPlayers(Game& game, int numPlayers);
void simulatePregameFlow(Game& game, int numPlayers);

#endif // TEST_UTILS_H
