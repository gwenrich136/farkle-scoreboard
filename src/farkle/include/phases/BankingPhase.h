#ifndef BankingPhase_h
#define BankingPhase_h

#include "GamePhase.h"

class BankingPhase : public InGamePhase {
public:
    virtual ~BankingPhase() = default;
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;

protected:
    virtual void updateAtRiskScoreDisplay(const GameState& state, const Displays& displays) override;
    virtual void updateProgressGrid(const GameState& state, const Displays& displays, bool includeAtRisk = true, bool blinkAtRisk = true) override;

private:
    float scoreMoveAccumulator;
};

#endif
