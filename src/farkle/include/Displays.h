#ifndef Displays_h
#define Displays_h

#include "ScoreDisplay.h"
#include "LedProgressGrid.h"
#include "FarkleWarningLights.h"
#include "TextDisplayV2.h"

// This struct groups all hardware display components to simplify
// passing them to the GamePhase::display() method.
struct Displays {
    ScoreDisplay& scoreDisplay;
    LedProgressGrid& grid;
    FarkleWarningLights& farkleLights;
    TextDisplayV2& textDisplay;

    Displays(ScoreDisplay& sd, LedProgressGrid& g, FarkleWarningLights& fl, TextDisplayV2& t)
        : scoreDisplay(sd), grid(g), farkleLights(fl), textDisplay(t) {}
};

#endif
