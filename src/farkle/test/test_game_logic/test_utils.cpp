#include "test_utils.h"
#include "Arduino.h"

void simulateButtonPress(Game& game, ButtonAction action, int count, unsigned long advance_time_millis) {
    for (int i = 0; i < count; ++i) {
        game.controlPad.press(action);
        advance_millis(advance_time_millis);
        game.loop();
    }
}

void simulateRotation(Game& game, int delta, unsigned long advance_time_millis) {
    game.controlPad.rotate(delta);
    advance_millis(advance_time_millis);
    game.loop();
}

void setupGameWithPlayers(Game& game, int numPlayers) {
    game.setup();

    // 1. Transition from TargetScoreSelectionPhase to PlayerSelectionPhase
    simulateButtonPress(game, ButtonAction::SELECT);
    game.loop();

    // Names from the pool to be consistent
    const char* names[] = {"Geewee", "Sammy", "Coach", "Sheshe", "Alex", "Tigre", "Pepa", "Fred", "Andrea"};
    for (int i = 0; i < numPlayers; ++i) {
        game.addPlayer(names[i]);
    }
    // 2. Transition from PlayerSelectionPhase to WaitingPhase
    simulateButtonPress(game, ButtonAction::FARKLE);
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
