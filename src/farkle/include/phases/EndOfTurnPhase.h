#ifndef EndOfTurnPhase_h
#define EndOfTurnPhase_h

#include "GamePhase.h"

class EndOfTurnPhase : public InGamePhase {
public:
    virtual ~EndOfTurnPhase() = default;
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) override;

protected:
    virtual void updateAtRiskScoreDisplay(const GameState& state, const Displays& displays) override;
    virtual void updateTextDisplay(const GameState& state, const Displays& displays) override;
};

#endif
