#ifndef FarkleWarningLights_h
#define FarkleWarningLights_h

#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include "PlayerLayout.h"

#define MAX_PLAYERS 8

class FarkleWarningLights
{
  public:
    FarkleWarningLights(int pin);
    void begin();
    void update(const int* farkleCounts, int playerCount, int currentPlayerIndex, bool isBlinking);

    // For compatibility with existing calls (e.g. resetGame)
    void farkle_state(int state);

    // Updated alternate signature
    void alternate(int currentPlayerIndex, int playerCount);

  private:
    Adafruit_NeoPixel _pixels;

    struct State {
        int farkleCounts[MAX_PLAYERS] = {0};
        int playerCount = 0;
        int currentPlayerIndex = -1;
        bool isBlinking = false;
        bool isDirty = true;

        bool operator==(const State& other) const {
            if (isDirty || other.isDirty ||
                playerCount != other.playerCount ||
                currentPlayerIndex != other.currentPlayerIndex ||
                isBlinking != other.isBlinking) {
                return false;
            }
            for (int i = 0; i < playerCount; ++i) {
                if (farkleCounts[i] != other.farkleCounts[i]) {
                    return false;
                }
            }
            return true;
        }
        bool operator!=(const State& other) const { return !(*this == other); }
    };

    State _lastState;

    void setPlayerPixels(int startRow, int numRows, uint32_t color);
};

#endif
