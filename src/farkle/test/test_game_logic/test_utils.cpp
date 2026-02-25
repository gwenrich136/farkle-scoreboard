#include "test_utils.h"
#include "Arduino.h"

void simulateButtonPress(Game& game, ButtonAction action, unsigned long advance_time_millis) {
    game.controlPad.press(action);
    advance_millis(advance_time_millis);
    game.loop();
}

void simulateRotation(Game& game, int delta, unsigned long advance_time_millis) {
    game.controlPad.rotate(delta);
    advance_millis(advance_time_millis);
    game.loop();
}

void simulateScore(Game& game, int points) {
    while (points >= 500) {
        simulateButtonPress(game, ButtonAction::PLUS_500);
        points -= 500;
    }
    while (points >= 100) {
        simulateButtonPress(game, ButtonAction::PLUS_100);
        points -= 100;
    }
    while (points >= 50) {
        simulateButtonPress(game, ButtonAction::PLUS_50);
        points -= 50;
    }
    // Remaining points by rotation (delta 1 = 50, but we assume points are multiple of 50 if using discrete buttons logic or rotation)
    // If < 50, cannot simulate perfectly with 50-step?
    // But rotation is 50/click.
    // If points < 50 and > 0, we can't do it.
    // Assuming tests use multiples of 50.
}

void setupGameWithPlayers(Game& game, int numPlayers) {
    game.setup();

    // 1. Transition from TargetScoreSelectionPhase to PlayerSelectionPhase
    simulateButtonPress(game, ButtonAction::FARKLE);

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
