#include "test_utils.h"
#include "Arduino.h"
#include "phases/WaitingPhase.h"

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

    // Directly populate players and skip the pregame phases.
    const char* names[] = {"Geewee", "Sammy", "Coach", "Sheshe", "Alex", "Tigre", "Pepa", "Fred", "Andrea"};
    for (int i = 0; i < numPlayers; ++i) {
        game.addPlayer(names[i]);
    }

    // Set target score explicitly
    game.setTargetScore(10000);

    // Enter WaitingPhase directly
    game.currentPhase = game.getPhase<WaitingPhase>();
    game.currentPhase->onEnter(game, game.state);

    // Mock MemoryCard behaviors that are usually initialized in PlayerSelectionPhase
    game.getMemoryCard().mock_getOrGenerateNextGameId_called = true;
    game.getMemoryCard().mock_initializeGameDirectory_called = true;
    game.getMemoryCard().mock_initializeGameDirectory_arg = 42;
    game.getMemoryCard().mock_writeGameMetadata_called = true;
    game.getMemoryCard().mock_writeGameMetadata_arg = 42;
    game.getMemoryCard().mock_setActiveGameId_called = true;
    game.getMemoryCard().mock_setActiveGameId_arg = 42;

    // Optional: do one game.loop() to ensure the internal displays and tracking are flushed.
    game.loop();
}

void simulatePregameFlow(Game& game, int numPlayers) {
    game.setup();

    // 0. Transition from StartupPhase to TargetScoreSelectionPhase
    simulateButtonPress(game, ButtonAction::SELECT);
    game.loop();

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
