#ifndef FarklingPhase_h
#define FarklingPhase_h

#include "GamePhase.h"

class FarklingPhase : public InGamePhase {
public:
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;

private:
    float scoreMoveAccumulator;
};

#endif
