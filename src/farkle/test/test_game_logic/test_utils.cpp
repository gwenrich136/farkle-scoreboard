#include "test_utils.h"
#include "Arduino.h"

void simulateButtonPress(Game& game, ButtonAction action, unsigned long advance_time_millis) {
    game.controlPad.press(action);
    advance_millis(advance_time_millis);
    game.loop();
}

void setupGameWithPlayers(Game& game, int numPlayers) {
    game.setup();

    // 1. Transition from TargetScoreSelectionPhase to PlayerSelectionPhase
    simulateButtonPress(game, ButtonAction::FARKLE);

    // Names from the pool to be consistent
    const char* names[] = {"Geewee", "Sammy", "Coach", "Sheshe", "Alex", "Tigre", "Pepa", "Fred", "Andrea"};
    for (int i = 0; i < numPlayers; ++i) {
        // Find the index in available names. Since we are adding from start, it's just i.
        // Actually, PlayerSelectionPhase filters them.
        // We can just simulate the BANK button if we want to be realistic,
        // but setupGameWithPlayers is meant to bypass selection.
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
