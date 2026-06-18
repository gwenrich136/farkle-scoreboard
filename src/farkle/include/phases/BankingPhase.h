#ifndef BankingPhase_h
#define BankingPhase_h

#include "GamePhase.h"

class BankingPhase : public InGamePhase {
public:
    virtual ~BankingPhase() = default;
    virtual void onEnter(Game& game, GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) override;

private:
    float scoreMoveAccumulator;
};

#endif
