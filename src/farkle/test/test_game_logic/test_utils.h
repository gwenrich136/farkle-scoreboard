#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "Game.h"
#include "ButtonActions.h"

void simulateButtonPress(Game& game, ButtonAction action, unsigned long advance_time_millis = 10);
void simulateNoAction(Game& game, unsigned long advance_time_millis = 10);
void waitForScoreAnimation(Game& game);

#endif // TEST_UTILS_H
