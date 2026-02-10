#include "GamePhase.h"

void PreGamePhase::display(const GameState& state, const Displays& displays) {
    // Explicitly clear ScoreDisplay and FarkleWarningLights in PreGamePhase
    displays.scoreDisplay.clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
    displays.scoreDisplay.clear(ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE);
    displays.scoreDisplay.clear(ScoreDisplay::DisplayType::COMPETITION_SCORE);
    displays.farkleLights.farkle_state(0);

    // Call hooks for subclasses
    updateProgressGrid(state, displays);
    updateTextDisplay(state, displays);
}
