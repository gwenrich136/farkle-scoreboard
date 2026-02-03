#ifndef PenaltyFarklingPhase_h
#define PenaltyFarklingPhase_h

#include "GamePhase.h"

enum class PenaltyStage {
    THE_PAIN,
    THE_DRAIN,
    THE_AFTERMATH
};

class PenaltyFarklingPhase : public InGamePhase {
public:
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;

protected:
    virtual void updateScoreDisplays(const GameState& state, const Displays& displays) override;
    virtual void updateWarningLights(const GameState& state, const Displays& displays) override;
    virtual void updateTextDisplay(const GameState& state, const Displays& displays) override;

private:
    PenaltyStage currentStage;
    unsigned long stageTimer;
    float scoreMoveAccumulator;
    unsigned long blinkTimer;
    bool isBlinking;
};

#endif
