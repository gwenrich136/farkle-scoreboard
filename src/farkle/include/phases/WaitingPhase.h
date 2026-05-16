#ifndef WaitingPhase_h
#define WaitingPhase_h

#include "GamePhase.h"

class WaitingPhase : public InGamePhase {
public:
    virtual ~WaitingPhase() = default;
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) override;

protected:
    virtual void updateAtRiskScoreDisplay(const GameState& state, const Displays& displays) override;
    virtual void updateWarningLights(const GameState& state, const Displays& displays) override;
    virtual int getBlinkingScore(const GameState& state) const override;

private:
    // Recomputes the ranked player list and sets the default competitor display.
    // Must be called whenever currentPlayerIndex changes (normal transition OR undo).
    void _recomputeLeaderboard(GameState& state);

    // Applies the undo operation: restores the previous player's state via MemoryCard
    // and re-syncs the leaderboard. A no-op if the journal is empty.
    void _applyUndo(Game& game, GameState& state);
};

#endif
