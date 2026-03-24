#ifndef GamePhase_h
#define GamePhase_h

#include "GameState.h"
#include "Input.h"
#include "Displays.h"
#include <vector>

// Forward declaration of the Game context class
class Game;

class GamePhase {
public:
    virtual ~GamePhase() {}

    // Called when the state machine transitions into this phase
    virtual void onEnter(GameState& state) = 0;

    // Called every loop to handle logic and transitions
    // Returns a pointer to the next phase (or 'this' to stay in current phase)
    virtual GamePhase* update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) = 0;

    // Called every loop to handle rendering
    // Note: GameState provides player hues (e.g., state.players[i].hue)
    // for rendering by specific phases (like PlayerSelectionPhase).
    virtual void display(const GameState& state, const Displays& displays) = 0;
};

/**
 * PreGamePhase
 *
 * Intermediate class for common pre-game logic and display behavior.
 * It ensures that primary gameplay displays (ScoreDisplay, FarkleWarningLights)
 * are explicitly cleared during setup phases.
 *
 * Responsibilities:
 * - Implement shared display logic for setup/menu phases.
 * - Provide virtual hooks for progress grid and text display updates.
 */
class PreGamePhase : public GamePhase {
public:
    virtual void display(const GameState& state, const Displays& displays) override;

protected:
    // Virtual hooks to allow overriding specific parts of the display
    virtual void updateProgressGrid(const GameState& state, const Displays& displays) {}
    virtual void updateTextDisplay(const GameState& state, const Displays& displays) {}
};

// Intermediate class for common in-game logic and display behavior
class InGamePhase : public GamePhase {
public:
    virtual void display(const GameState& state, const Displays& displays) override;

protected:
    // Virtual hooks to allow overriding specific parts of the display
    virtual void updateScoreDisplays(const GameState& state, const Displays& displays);
    virtual void updateAtRiskScoreDisplay(const GameState& state, const Displays& displays);
    virtual void updateCurrentPlayerScoreDisplay(const GameState& state, const Displays& displays);
    virtual void updateCompetitionScoreDisplay(const GameState& state, const Displays& displays);

    virtual void updateProgressGrid(const GameState& state, const Displays& displays);
    virtual void updateWarningLights(const GameState& state, const Displays& displays);
    virtual void updateTextDisplay(const GameState& state, const Displays& displays);

    // Virtual hooks for calculating scores displayed on the Progress Grid
    virtual int getGridScoreForPlayer(const GameState& state, int playerIndex) const;
    virtual int getBlinkingScore(const GameState& state) const;

    // Helper to calculate the highest score among all players
    int calculateLeadingScore(const GameState& state);

    // Helper to advance to the next player
    void endTurn(GameState& state);

    // Reusable vector for scores to avoid repeated allocations
    std::vector<int> m_scores;
    uint32_t m_lastScoresVersion = 0;
};

#endif
