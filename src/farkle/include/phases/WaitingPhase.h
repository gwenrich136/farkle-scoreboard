#ifndef WaitingPhase_h
#define WaitingPhase_h

#include "GamePhase.h"

class WaitingPhase : public InGamePhase {
public:
    virtual ~WaitingPhase() = default;
    virtual void onEnter(Game& game, GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;
};

#endif
