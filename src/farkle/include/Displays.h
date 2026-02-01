#ifndef Displays_h
#define Displays_h

#include "ScoreDisplay.h"
#include "LedProgressGrid.h"
#include "FarkleWarningLights.h"
#include "TextDisplay.h"

// This struct groups all hardware display components to simplify
// passing them to the GamePhase::display() method.
struct Displays {
    ScoreDisplay& scoreDisplay;
    LedProgressGrid& grid;
    FarkleWarningLights& farkleLights;
    TextDisplay& oled;

    Displays(ScoreDisplay& sd, LedProgressGrid& g, FarkleWarningLights& fl, TextDisplay& t)
        : scoreDisplay(sd), grid(g), farkleLights(fl), oled(t) {}
};

#endif
