#include "phases/PenaltyFarklingPhase.h"
#include "Game.h"
#include <algorithm>

// Constants for animation sequence
const unsigned long PAIN_DURATION = 5000;
const float PENALTY_DRAIN_SPEED = 1.0f;

void PenaltyFarklingPhase::onEnter(GameState& state) {
    currentStage = PenaltyStage::THE_PAIN;
    stageTimer = 0;
    scoreMoveAccumulator = 0.0f;

    // Calculate penalty: min(1000, player.score)
    state.atRiskScore = -1 * std::min(1000, state.players[state.currentPlayerIndex].score);

    // Reset the farkle count immediately
    state.players[state.currentPlayerIndex].farkle_count = 0;
}

GamePhase* PenaltyFarklingPhase::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    stageTimer += deltaTime;

    switch (currentStage) {
        case PenaltyStage::THE_PAIN:
            // Dramatic pause for 5 seconds with blinking score
            if (stageTimer >= PAIN_DURATION) {
                currentStage = PenaltyStage::THE_DRAIN;
            }
            break;

        case PenaltyStage::THE_DRAIN:
            // Perform Animation (Inverse banking)
            if (state.atRiskScore < 0) {
                scoreMoveAccumulator += (PENALTY_DRAIN_SPEED * deltaTime);
                int pointsToSubtract = (int)scoreMoveAccumulator;

                if (pointsToSubtract > 0) {
                    // Ensure we don't subtract more than what's left in atRiskScore (a negative value)
                    if (pointsToSubtract > -state.atRiskScore) {
                        pointsToSubtract = -state.atRiskScore;
                    }

                    state.addPlayerScore(state.currentPlayerIndex, -pointsToSubtract);
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
            if (input.action != ButtonAction::NONE) {
                this->endTurn(state);
                return game.getPhase<WaitingPhase>();
            }
            break;
    }

    return this;
}

void PenaltyFarklingPhase::updateAtRiskScoreDisplay(const GameState& state, const Displays& displays) {
    // Use the blink parameter provided by the updated ScoreDisplay library
    bool shouldBlink = (currentStage == PenaltyStage::THE_PAIN);

    if (state.atRiskScore == 0) {
        displays.scoreDisplay.clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
    } else {
        displays.scoreDisplay.print_number(state.atRiskScore, ScoreDisplay::DisplayType::AT_RISK_SCORE, shouldBlink);
    }
}

void PenaltyFarklingPhase::updateWarningLights(const GameState& state, const Displays& displays) {
    // Lights alternate during THE_PAIN and THE_DRAIN stages
    if (currentStage == PenaltyStage::THE_PAIN || currentStage == PenaltyStage::THE_DRAIN) {
        displays.farkleLights.alternate(state.currentPlayerIndex, state.players.size());
    } else {
        // Inherit behavior for THE_AFTERMATH (turns them off as count is 0)
        InGamePhase::updateWarningLights(state, displays);
    }
}

void PenaltyFarklingPhase::updateTextDisplay(const GameState& state, const Displays& displays) {
    // Show penalty quip throughout the sequence
    displays.oled.print("CATASTROPHIC FARKLE!");
}
