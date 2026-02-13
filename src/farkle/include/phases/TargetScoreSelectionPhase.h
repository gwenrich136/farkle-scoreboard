#ifndef TargetScoreSelectionPhase_h
#define TargetScoreSelectionPhase_h

#include "GamePhase.h"

/**
 * TargetScoreSelectionPhase
 *
 * Allows users to define the goal of the game before adding players.
 * Navigates through target score options in increments of 1,000.
 */
class TargetScoreSelectionPhase : public PreGamePhase {
public:
    virtual ~TargetScoreSelectionPhase() = default;
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;

protected:
    virtual void updateProgressGrid(const GameState& state, const Displays& displays) override;
    virtual void updateTextDisplay(const GameState& state, const Displays& displays) override;
};

#endif
