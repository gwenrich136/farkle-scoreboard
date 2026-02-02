#include "phases/PenaltyFarklePhase.h"
#include "Game.h"
#include <Arduino.h>

// Constants for animation
const float PENALTY_DRAIN_SPEED = 0.5f; // points per millisecond (approx 500 points/sec)

void PenaltyFarklePhase::onEnter(GameState& state) {
    phaseStartTime = millis();
    scoreMoveAccumulator = 0;

    // Apply the penalty
    state.players[state.currentPlayerIndex].score -= PENALTY_AMOUNT;
    if (state.players[state.currentPlayerIndex].score < 0) {
        state.players[state.currentPlayerIndex].score = 0;
    }
}

GamePhase* PenaltyFarklePhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    unsigned long elapsedTime = millis() - phaseStartTime;

    if (elapsedTime < FLASH_DURATION) {
        // Flashing period
        game.farkleLights.alternate();
    } else {
        // After flashing, wait for dismissal
        if (action != ButtonAction::NONE) {
            this->endTurn(state);
            return game.getPhase<WaitingPhase>();
        }
    }
    return this;
}

void PenaltyFarklePhase::display(const GameState& state, const Displays& displays) {
    unsigned long elapsedTime = millis() - phaseStartTime;

    // Determine if the display should be blank or show the penalty
    bool flashIsOn = (elapsedTime / 250) % 2 == 0;

    if (elapsedTime < FLASH_DURATION && flashIsOn) {
        displays.scoreDisplay.print_number(-PENALTY_AMOUNT, 0); // Show penalty on at-risk display
    } else {
        displays.scoreDisplay.clear_display(0); // Blank the at-risk display
    }

    // Continue to show the (now penalized) banked score
    InGamePhase::display(state, displays);
}
