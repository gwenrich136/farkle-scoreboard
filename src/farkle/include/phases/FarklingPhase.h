#ifndef FarklingPhase_h
#define FarklingPhase_h

#include "GamePhase.h"

class FarklingPhase : public InGamePhase {
public:
    virtual ~FarklingPhase() = default;
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;

protected:
    virtual void updateAtRiskScoreDisplay(const GameState& state, const Displays& displays) override;
    virtual void updateCurrentPlayerScoreDisplay(const GameState& state, const Displays& displays) override;
    virtual void updateProgressGrid(const GameState& state, const Displays& displays, bool includeAtRisk = true, bool blinkAtRisk = true) override;

private:
    float scoreMoveAccumulator;
};

#endif
