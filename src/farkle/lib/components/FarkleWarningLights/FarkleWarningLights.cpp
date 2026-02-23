#include "FarkleWarningLights.h"

#define NUM_LEDS 8

FarkleWarningLights::FarkleWarningLights(int pin)
  : _pixels(NUM_LEDS, pin, NEO_GRB + NEO_KHZ800)
{
    _lastState.isDirty = true;
}

void FarkleWarningLights::begin() {
    _pixels.begin();
    _pixels.show(); // Clear
}

void FarkleWarningLights::farkle_state(int state) {
    if (state == 0) {
        _pixels.clear();
        _pixels.show();
        _lastState.playerCount = 0; // Reset state tracking
        _lastState.isDirty = true;
    }
}

void FarkleWarningLights::update(const int* farkleCounts, int playerCount, int currentPlayerIndex, bool isBlinking) {
    if (playerCount > MAX_PLAYERS) playerCount = MAX_PLAYERS;

    // Check if state changed
    State newState;
    newState.playerCount = playerCount;
    newState.currentPlayerIndex = currentPlayerIndex;
    newState.isBlinking = isBlinking;
    newState.isDirty = false;
    for (int i = 0; i < playerCount; ++i) {
        newState.farkleCounts[i] = farkleCounts[i];
    }

    if (newState == _lastState) {
        return;
    }

    _lastState = newState;
    _lastState.isDirty = false;

    _pixels.clear();

    for (int i = 0; i < playerCount; ++i) {
        PlayerRows rows = PlayerLayout::getMapping(playerCount, i);

        int farkles = farkleCounts[i];
        bool isActive = (i == currentPlayerIndex);

        uint32_t color = 0;

        if (isActive) {
            // Active Player
            // 0 Farkles: White (Full, Blink)
            // 1 Farkle: Yellow (Full? No, requirement says "Active Player: Full Brightness")
            // 2+ Farkles: Red (Full)

            if (isBlinking) { // Blink ON phase
                if (farkles == 0) {
                    color = _pixels.ColorHSV(0, 0, 255); // White Full
                } else if (farkles == 1) {
                    // Yellow Full (Hue ~10922)
                    color = _pixels.ColorHSV(10922, 255, 255);
                } else { // 2+
                    // Red Full (Hue 0)
                    color = _pixels.ColorHSV(0, 255, 255);
                }
            } else {
                // Blink OFF phase -> OFF
                color = 0;
            }
        } else {
            // Idle Player
            // 0 Farkles: OFF
            // 1 Farkle: Yellow (Dim 50)
            // 2+ Farkles: Red (Dim 50)

            if (farkles == 0) {
                color = 0;
            } else if (farkles == 1) {
                color = _pixels.ColorHSV(10922, 255, 50);
            } else {
                color = _pixels.ColorHSV(0, 255, 50);
            }
        }

        if (color != 0) {
            setPlayerPixels(rows.startRow, rows.numRows, color);
        }
    }

    _pixels.show();
}

void FarkleWarningLights::setPlayerPixels(int startRow, int numRows, uint32_t color) {
    for (int r = 0; r < numRows; ++r) {
        int ledIndex = startRow + r;
        if (ledIndex < NUM_LEDS) {
            _pixels.setPixelColor(ledIndex, color);
        }
    }
}

void FarkleWarningLights::alternate(int currentPlayerIndex) {
    // Deprecated, no-op or handled by update
}
