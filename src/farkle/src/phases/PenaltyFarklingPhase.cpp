#include "phases/PenaltyFarklingPhase.h"
#include "Game.h"
#include <algorithm>

// Constants for animation sequence
const unsigned long PAIN_DURATION = 3000;
const unsigned long BLINK_INTERVAL = 250;
const float PENALTY_DRAIN_SPEED = 1.0f;

void PenaltyFarklingPhase::onEnter(GameState& state) {
    currentStage = PenaltyStage::THE_PAIN;
    stageTimer = 0;
    scoreMoveAccumulator = 0.0f;
    blinkTimer = 0;
    isBlinking = false;

    // Calculate penalty: min(1000, player.score)
    // atRiskScore is set to the negative penalty amount
    state.atRiskScore = -1 * std::min(1000, state.players[state.currentPlayerIndex].score);

    // Reset the farkle count immediately
    state.players[state.currentPlayerIndex].farkle_count = 0;
}

GamePhase* PenaltyFarklingPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    stageTimer += deltaTime;

    switch (currentStage) {
        case PenaltyStage::THE_PAIN:
            blinkTimer += deltaTime;
            if (blinkTimer >= BLINK_INTERVAL) {
                isBlinking = !isBlinking;
                blinkTimer = 0;
            }
            if (stageTimer >= PAIN_DURATION) {
                currentStage = PenaltyStage::THE_DRAIN;
                isBlinking = false; // Solid during drain
            }
            break;

        case PenaltyStage::THE_DRAIN:
            if (state.atRiskScore < 0) {
                scoreMoveAccumulator += (PENALTY_DRAIN_SPEED * deltaTime);
                int pointsToSubtract = (int)scoreMoveAccumulator;

                if (pointsToSubtract > 0) {
                    Player& currentPlayer = state.players[state.currentPlayerIndex];

                    // Ensure we don't subtract more than what's left in atRiskScore (a negative value)
                    if (pointsToSubtract > -state.atRiskScore) {
                        pointsToSubtract = -state.atRiskScore;
                    }

                    currentPlayer.score -= pointsToSubtract;
                    state.atRiskScore += pointsToSubtract;
                    scoreMoveAccumulator -= (float)pointsToSubtract;
                }
            }

            if (state.atRiskScore >= 0) {
                state.atRiskScore = 0; // Clean up any fractional remainder
                currentStage = PenaltyStage::THE_AFTERMATH;
            }
            break;

        case PenaltyStage::THE_AFTERMATH:
            // Wait for user dismissal
            if (action != ButtonAction::NONE) {
                this->endTurn(state);
                return game.getPhase<WaitingPhase>();
            }
            break;
    }

    return this;
}

void PenaltyFarklingPhase::updateScoreDisplays(const GameState& state, const Displays& displays) {
    int leadingScore = calculateLeadingScore(state);

    // At Risk Score (Device 0) flashes during THE_PAIN
    if (currentStage == PenaltyStage::THE_PAIN && isBlinking) {
        displays.scoreDisplay.clear(0);
    } else {
        displays.scoreDisplay.print_number(state.atRiskScore, 0);
    }

    displays.scoreDisplay.print_number(state.players[state.currentPlayerIndex].score, 1);
    displays.scoreDisplay.print_number(leadingScore, 2);
}

void PenaltyFarklingPhase::updateWarningLights(const GameState& state, const Displays& displays) {
    // Lights alternate during THE_PAIN and THE_DRAIN
    if (currentStage == PenaltyStage::THE_PAIN || currentStage == PenaltyStage::THE_DRAIN) {
        displays.farkleLights.alternate();
    } else {
        // Lights OFF during THE_AFTERMATH
        displays.farkleLights.farkle_state(0);
    }
}

void PenaltyFarklingPhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    // Show penalty quip throughout the sequence
    displays.oled.print("CATASTROPHIC FARKLE!");
}
