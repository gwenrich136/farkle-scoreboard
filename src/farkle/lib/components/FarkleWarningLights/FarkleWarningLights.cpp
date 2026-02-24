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

void FarkleWarningLights::update(const int* farkleCounts, int playerCount, int blinkingPlayerIndex, bool isBlinking) {
    if (playerCount > MAX_PLAYERS) playerCount = MAX_PLAYERS;

    // Check if state changed
    State newState;
    newState.playerCount = playerCount;
    newState.blinkingPlayerIndex = blinkingPlayerIndex;
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
        bool isActive = (i == blinkingPlayerIndex);

        uint32_t color = 0;

        // Brightness for all active LEDs is now 50% (128)
        uint8_t brightness = 128;

        if (isActive) {
            // Active Player (Blinking Turn Indicator)
            // 0 Farkles: White
            // 1 Farkle: Yellow
            // 2+ Farkles: Red

            if (isBlinking) { // Blink ON phase
                if (farkles == 0) {
                    color = _pixels.ColorHSV(0, 0, brightness); // White
                } else if (farkles == 1) {
                    color = _pixels.ColorHSV(10922, 255, brightness); // Yellow
                } else {
                    color = _pixels.ColorHSV(0, 255, brightness); // Red
                }
            } else {
                // Blink OFF phase -> OFF
                color = 0;
            }
        } else {
            // Idle Player (Solid)
            // 0 Farkles: OFF
            // 1 Farkle: Yellow
            // 2+ Farkles: Red

            if (farkles == 0) {
                color = 0;
            } else if (farkles == 1) {
                color = _pixels.ColorHSV(10922, 255, brightness); // Yellow
            } else {
                color = _pixels.ColorHSV(0, 255, brightness); // Red
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

void FarkleWarningLights::alternate(int currentPlayerIndex, int playerCount) {
    // Determine cycle time (1000ms loop)
    unsigned long time = millis() % 1000;

    uint16_t hue = 0; // Red

    if (time < 400) {
        // Red
        hue = 0;
    } else if (time < 500) {
        // Transition Red -> Yellow (0 -> 10922)
        hue = map(time, 400, 500, 0, 10922);
    } else if (time < 900) {
        // Yellow
        hue = 10922;
    } else {
        // Transition Yellow -> Red (10922 -> 0)
        hue = map(time, 900, 1000, 10922, 0);
    }

    // Use Full brightness (255) for the "Pain" animation
    uint32_t color = _pixels.ColorHSV(hue, 255, 255);

    // Light up current player
    PlayerRows rows = PlayerLayout::getMapping(playerCount, currentPlayerIndex);

    // Clear all first to ensure clean state
    _pixels.clear();
    setPlayerPixels(rows.startRow, rows.numRows, color);
    _pixels.show();

    // Invalidate state so next regular update refreshes correctly
    _lastState.isDirty = true;
}
