#ifndef StartupPhase_h
#define StartupPhase_h

#include "GamePhase.h"

class StartupPhase : public PreGamePhase {
public:
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) override;

protected:
    virtual void updateTextDisplay(const GameState& state, const Displays& displays) override;
};

#endif
