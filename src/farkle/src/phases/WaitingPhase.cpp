#include "phases/WaitingPhase.h"
#include "Game.h"

void WaitingPhase::onEnter(GameState& state) {
    // No specific local state to reset for WaitingPhase
}

GamePhase* WaitingPhase::update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) {
    // 1. Check for Game End
    // If the final round was triggered and it's now back to a player who has 5000+, the game ends.
    if (state.finalRoundTriggered && state.players[state.currentPlayerIndex].score >= state.targetScore) {
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
                Player& currentPlayer = state.players[state.currentPlayerIndex];
                if (currentPlayer.farkle_count >= 2) {
                    return game.getPhase<PenaltyFarklingPhase>();
                } else {
                    return game.getPhase<FarklingPhase>();
                }
            }
            break;

        default:
            break;
    }

    return this;
}
