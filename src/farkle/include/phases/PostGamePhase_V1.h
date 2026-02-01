#ifndef PostGamePhase_V1_h
#define PostGamePhase_V1_h

#include "GamePhase.h"

class PostGamePhase_V1 : public GamePhase {
public:
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;
    virtual void display(const GameState& state, const Displays& displays) override;
};

#endif
