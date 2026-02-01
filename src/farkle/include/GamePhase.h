#ifndef GamePhase_h
#define GamePhase_h

#include "GameState.h"
#include "ButtonActions.h"
#include "Displays.h"

// Forward declaration of the Game context class
class Game;

class GamePhase {
public:
    virtual ~GamePhase() {}

    // Called when the state machine transitions into this phase
    virtual void onEnter(GameState& state) = 0;

    // Called every loop to handle logic and transitions
    // Returns a pointer to the next phase (or 'this' to stay in current phase)
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) = 0;

    // Called every loop to handle rendering
    virtual void display(const GameState& state, const Displays& displays) = 0;
};

// Intermediate class for common in-game logic and display behavior
class InGamePhase : public GamePhase {
public:
    virtual void display(const GameState& state, const Displays& displays) override;

protected:
    // Helper to advance to the next player
    void endTurn(GameState& state);
};

#endif
