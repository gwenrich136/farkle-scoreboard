#ifndef WaitingPhase_h
#define WaitingPhase_h

#include "GamePhase.h"

class WaitingPhase : public InGamePhase {
public:
    virtual ~WaitingPhase() = default;
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;

protected:
    virtual void updateAtRiskScoreDisplay(const GameState& state, const Displays& displays) override;
    virtual void updateProgressGrid(const GameState& state, const Displays& displays, bool includeAtRisk = true, bool blinkAtRisk = true) override;
    virtual void updateWarningLights(const GameState& state, const Displays& displays) override;
};

#endif
