#ifndef FarklingPhase_h
#define FarklingPhase_h

#include "GamePhase.h"

class FarklingPhase : public InGamePhase {
public:
    virtual ~FarklingPhase() = default;
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) override;

protected:
    virtual int getGridScoreForPlayer(const GameState& state, int playerIndex) const override;

private:
    float scoreMoveAccumulator;
};

#endif
