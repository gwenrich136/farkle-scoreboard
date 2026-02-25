#include "phases/WaitingPhase.h"
#include "Game.h"
#include <Arduino.h>

// Ensure MAX_PLAYERS is available if not already defined (it is in FarkleWarningLights.h)
#ifndef MAX_PLAYERS
#define MAX_PLAYERS 8
#endif

void WaitingPhase::onEnter(GameState& state) {
    // No specific local state to reset for WaitingPhase
}

GamePhase* WaitingPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    // 1. Check for Game End
    // If the final round was triggered and it's now back to a player who has reached the target score, the game ends.
    if (state.finalRoundTriggered && state.players.size() > 0 && state.players[state.currentPlayerIndex].score >= state.targetScore) {
        return game.getPhase<PostGamePhase_V1>();
    }

    // 2. Handle Scoring Inputs
    switch (action) {
        case UP_1000:  state.atRiskScore += 1000; break;
        case RIGHT_500: state.atRiskScore += 500;  break;
        case LEFT_100:  state.atRiskScore += 100;  break;
        case DOWN_50:   state.atRiskScore += 50;   break;
        case CLEAR:     state.atRiskScore = 0;     break;
        
        case BANK:
            if (state.atRiskScore > 0) {
                return game.getPhase<BankingPhase>();
            }
            break;
            
        case FARKLE:
            {
                if (state.players.empty()) break;
                Player& currentPlayer = state.players[state.currentPlayerIndex];
                return (currentPlayer.farkle_count >= 2) ? (GamePhase*)game.getPhase<PenaltyFarklingPhase>() : (GamePhase*)game.getPhase<FarklingPhase>();
            }
            break;

        default:
            break;
    }

    return this;
}

void WaitingPhase::updateAtRiskScoreDisplay(const GameState& state, const Displays& displays) {
    displays.scoreDisplay.print_number(state.atRiskScore, ScoreDisplay::DisplayType::AT_RISK_SCORE);
}

void WaitingPhase::updateWarningLights(const GameState& state, const Displays& displays) {
    int farkleCounts[MAX_PLAYERS];
    int count = 0;
    for (const auto& player : state.players) {
        if (count < MAX_PLAYERS) {
            farkleCounts[count++] = player.farkle_count;
        }
    }

    // Sync with LedProgressGrid blink logic (500ms half period)
    bool isBlinkOn = (millis() % 1000) > 500;

    // In WaitingPhase, we WANT the current player to blink (turn indicator)
    displays.farkleLights.update(farkleCounts, count, state.currentPlayerIndex, isBlinkOn);
}
