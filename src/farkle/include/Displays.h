#ifndef Displays_h
#define Displays_h

#include "ScoreDisplay.h"
#include "LedProgressGrid.h"
#include "FarkleWarningLights.h"
#include "TextDisplayV2.h"
#include "SoundPlayer.h"

// This struct groups all hardware display components to simplify
// passing them to the GamePhase::display() method.
struct Displays {
    ScoreDisplay& scoreDisplay;
    LedProgressGrid& grid;
    FarkleWarningLights& farkleLights;
    TextDisplayV2& textDisplay;
    SoundPlayer& soundPlayer;

    Displays(ScoreDisplay& sd, LedProgressGrid& g, FarkleWarningLights& fl, TextDisplayV2& t, SoundPlayer& sp)
        : scoreDisplay(sd), grid(g), farkleLights(fl), textDisplay(t), soundPlayer(sp) {}
};

#endif
