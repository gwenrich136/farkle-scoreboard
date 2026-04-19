#include "phases/WaitingPhase.h"
#include "Game.h"
#include <Arduino.h>
#include "GameConstants.h"
#include <algorithm>

void WaitingPhase::onEnter(GameState& state) {
    // Recompute the ranking of all players
    // Populate the list once if empty
    if (state.rankedPlayerIndices.empty()) {
        for (size_t i = 0; i < state.players.size(); ++i) {
            state.rankedPlayerIndices.push_back(i);
        }
    }

    // Sort the list based on current scores
    // Secondary tie-breaker: turns away from current player (ascending)
    std::sort(state.rankedPlayerIndices.begin(), state.rankedPlayerIndices.end(),
              [&state](int a, int b) {
                  if (state.players[a].score != state.players[b].score) {
                      return state.players[a].score > state.players[b].score;
                  }
                  int numPlayers = (int)state.players.size();
                  int turnsAwayA = (a - state.currentPlayerIndex + numPlayers) % numPlayers;
                  int turnsAwayB = (b - state.currentPlayerIndex + numPlayers) % numPlayers;
                  return turnsAwayA < turnsAwayB;
              });

    // Set competitor rank to 0, or 1 if 0 is the current player (assuming > 1 player)
    state.currentCompetitorRank = 0;
    if (!state.rankedPlayerIndices.empty() && state.rankedPlayerIndices[0] == state.currentPlayerIndex) {
        if (state.rankedPlayerIndices.size() > 1) {
            state.currentCompetitorRank = 1;
        }
    }
}

GamePhase* WaitingPhase::update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) {
    // 1. Check for Game End
    // If the final round was triggered and it's now back to a player who has reached the target score, the game ends.
    if (state.finalRoundTriggered && state.players.size() > 0 && state.players[state.currentPlayerIndex].score >= state.targetScore) {
        return game.getPhase<PostGamePhase_V1>();
    }

    if (input.rotationDelta != 0 && state.rankedPlayerIndices.size() > 1) {
        int listSize = state.rankedPlayerIndices.size();

        // Apply the full rotation delta
        state.currentCompetitorRank = (state.currentCompetitorRank + input.rotationDelta) % listSize;
        if (state.currentCompetitorRank < 0) {
            state.currentCompetitorRank += listSize;
        }

    }

    switch (input.action) {
        case ButtonAction::PLUS_500:
            state.atRiskScore += 500;
            break;
        case ButtonAction::PLUS_100:
            state.atRiskScore += 100;
            break;
        case ButtonAction::PLUS_50:
            state.atRiskScore += 50;
            break;
        case ButtonAction::CLEAR:
            state.atRiskScore = 0;
            break;
        
        case ButtonAction::BANK:
            if (state.atRiskScore > 0) {
                return game.getPhase<BankingPhase>();
            }
            break;
            
        case ButtonAction::FARKLE:
            {
                if (state.players.empty()) break;

                // If farkle_count >= 2 (already), entering here makes it 3 (Catastrophic).
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

int WaitingPhase::getBlinkingScore(const GameState& state) const {
    return state.atRiskScore;
}
