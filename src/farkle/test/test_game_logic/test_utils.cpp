#include "test_utils.h"
#include "Arduino.h"

void simulateButtonPress(Game& game, ButtonAction action, unsigned long advance_time_millis) {
    game.controlPad.press(action);
    advance_millis(advance_time_millis);
    game.loop();
}

void waitForScoreAnimation(Game& game) {
    while (game.state.atRiskScore != 0) {
        simulateNoAction(game);
    }
}

void simulateNoAction(Game& game, unsigned long advance_time_millis) {
    advance_millis(advance_time_millis);
    game.loop();
}
