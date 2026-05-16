#ifndef StartupPhase_h
#define StartupPhase_h

#include "GamePhase.h"

class StartupPhase : public PreGamePhase {
public:
    StartupPhase() : _selectionIndex(0), _hasActiveGame(false) {}
    virtual void onEnter(GameState& state) override;
    virtual void onEnter(Game& game, GameState& state);
    virtual GamePhase* update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) override;

protected:
    virtual void updateTextDisplay(const GameState& state, const Displays& displays) override;

private:
    int _selectionIndex;
    bool _hasActiveGame;
};

#endif
