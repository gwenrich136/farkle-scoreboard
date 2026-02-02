#ifndef PenaltyFarklePhase_h
#define PenaltyFarklePhase_h

#include "GamePhase.h"

class PenaltyFarklePhase : public InGamePhase {
public:
    virtual ~PenaltyFarklePhase() = default;
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;
    virtual void display(const GameState& state, const Displays& displays) override;

private:
    unsigned long phaseStartTime;
    float scoreMoveAccumulator;
    const int FLASH_DURATION = 3000; // ms
    const int PENALTY_AMOUNT = 1000;
};

#endif
