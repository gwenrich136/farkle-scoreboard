#ifndef PlayerSelectionPhase_h
#define PlayerSelectionPhase_h

#include "GamePhase.h"
#include <vector>
#include <string>

/**
 * PlayerSelectionPhase
 *
 * This phase allows users to build the game roster by selecting names from
 * a predefined pool. It manages menu navigation, real-time filtering of
 * already-added names, and enforces roster limits.
 *
 * Responsibilities:
 * - Navigate the filtered name pool using ControlPad buttons.
 * - Add players to the GameState and update hardware color assignments.
 * - Enforce the 8-player maximum roster size.
 * - Gate the transition to the gameplay loop (min 1 player).
 */
class PlayerSelectionPhase : public PreGamePhase {
public:
    virtual ~PlayerSelectionPhase() = default;
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) override;

protected:
    virtual void updateProgressGrid(const GameState& state, const Displays& displays) override;
    virtual void updateTextDisplay(const GameState& state, const Displays& displays) override;

private:
    char m_currentSelection[13];
};

#endif
