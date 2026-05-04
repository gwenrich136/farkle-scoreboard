#include "phases/WaitingPhase.h"
#include "Game.h"
#include <Arduino.h>
#include "GameConstants.h"
#include <algorithm>

void WaitingPhase::onEnter(GameState& state) {
    _recomputeLeaderboard(state);
}

void WaitingPhase::_applyUndo(Game& game, GameState& state) {
    auto undoResult = game.getMemoryCard().undoLastTurn();
    if (!undoResult.success) return; // Journal empty — nothing to undo

    int numPlayers = (int)state.players.size();
    state.currentPlayerIndex = (state.currentPlayerIndex - 1 + numPlayers) % numPlayers;
    state.updatePlayerScore(undoResult.playerIndex, undoResult.previousScore);
    state.players[undoResult.playerIndex].farkle_count = undoResult.previousFarkleCount;
    state.atRiskScore = 0;
    // Re-rank and reset competitor display, same as a normal turn transition
    _recomputeLeaderboard(state);
}

void WaitingPhase::_recomputeLeaderboard(GameState& state) {
    // Populate the index list once if empty
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

    // Default competitor to rank 0, unless that IS the current player (show rank 1 instead)
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

        case ButtonAction::UNDO:
            _applyUndo(game, state);
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
