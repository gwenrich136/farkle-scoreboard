#ifndef LedProgressGrid_h
#define LedProgressGrid_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <optional>
#include "PlayerLayout.h"
#include "GameConstants.h"

class LedProgressGrid {
public:
  enum class DisplayMode {
    NONE,
    IN_GAME,
    PRE_GAME
  };

  struct State {
    DisplayMode mode = DisplayMode::NONE;
    int scores[MAX_PLAYERS] = {0};
    int currentPlayerIndex = -1;
    int blinkingScore = -1;
    bool isBlinkOn = false;
    bool isPlayerPending = false;
    int playerCount = 0;
    int maxScore = 0;
    bool isDirty = true;

    bool operator==(const State& other) const {
      if (isDirty || other.isDirty ||
          mode != other.mode ||
          currentPlayerIndex != other.currentPlayerIndex ||
          blinkingScore != other.blinkingScore ||
          isBlinkOn != other.isBlinkOn ||
          isPlayerPending != other.isPlayerPending ||
          playerCount != other.playerCount ||
          maxScore != other.maxScore) {
        return false;
      }
      for (int i = 0; i < playerCount; ++i) {
        if (scores[i] != other.scores[i]) {
          return false;
        }
      }
      return true;
    }
    bool operator!=(const State& other) const { return !(*this == other); }
  };

  LedProgressGrid(uint8_t pin);
  void begin();
  void setTargetScore(int target);
  int addPlayer(uint16_t hue);
  bool isMaxPlayersReached();
  void reset();
  void clear();
  void update(const int* scores, int playerCount, int currentPlayerIndex, int blinkingScore);
  void displayPlayersPregame(std::optional<uint16_t> pendingPlayerHue);
  int getMaxScore() const { return _maxScore; }

private:
  Adafruit_NeoPixel _pixels;

  int _playerCount;
  uint16_t _playerHues[MAX_PLAYERS];
  int _maxScore;
  int _targetScore;
  bool _isBlinkOn;

  State _lastState;

  void illuminate_row(int row, uint16_t hue, float ratio, uint8_t brightness = 255);
  int get_pixel_index(int row, int col);
  int getRemainderBrightness(float remainder, int fullBrightness);

  bool shouldRefresh(const State& newState);
  void renderPlayerRows(PlayerRows rows, uint16_t hue, float ratio, uint8_t brightness);
};

#endif
