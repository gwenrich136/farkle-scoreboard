#include "test_utils.h"
#include "Arduino.h"

void simulateButtonPress(Game& game, ButtonAction action, unsigned long advance_time_millis) {
    game.controlPad.press(action);
    advance_millis(advance_time_millis);
    game.loop();
}

void setupGameWithPlayers(Game& game, int numPlayers) {
    game.setup();
    // Assuming we start in PlayerSelectionPhase
    for (int i = 0; i < numPlayers; ++i) {
        // Just add players directly for testing convenience, or simulate buttons
        char name[20];
        sprintf(name, "PLAYER %d", i + 1);
        game.addPlayer(name);
    }
    // Transition to WaitingPhase
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
