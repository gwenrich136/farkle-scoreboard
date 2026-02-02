#ifndef PenaltyFarklingPhase_h
#define PenaltyFarklingPhase_h

#include "GamePhase.h"

class PenaltyFarklingPhase : public InGamePhase {
public:
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;

private:
    float scoreMoveAccumulator;
};

#endif
