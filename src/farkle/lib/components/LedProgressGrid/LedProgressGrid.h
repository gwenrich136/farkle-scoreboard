#ifndef LedProgressGrid_h
#define LedProgressGrid_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <vector> // Required for std::vector

#define MAX_PLAYERS 8

class LedProgressGrid {
public:
  enum class DisplayMode {
    NONE,
    IN_GAME,
    PRE_GAME
  };

  struct PlayerRows {
    int startRow;
    int numRows;
  };

  struct State {
    DisplayMode mode = DisplayMode::NONE;
    int scores[MAX_PLAYERS] = {0};
    int currentPlayerIndex = -1;
    int atRiskScore = -1;
    bool isBlinkOn = false;
    bool isPlayerPending = false;
    int playerCount = 0;
    bool isDirty = true;

    bool operator==(const State& other) const {
      if (isDirty || other.isDirty ||
          mode != other.mode ||
          currentPlayerIndex != other.currentPlayerIndex ||
          atRiskScore != other.atRiskScore ||
          isBlinkOn != other.isBlinkOn ||
          isPlayerPending != other.isPlayerPending ||
          playerCount != other.playerCount) {
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
  int addPlayer();
  bool isMaxPlayersReached();
  void reset();
  void clear();
  void update(const std::vector<int>& scores, int currentPlayerIndex, int atRiskScore);
  void displayPlayersPregame(bool isPlayerPending);

private:
  Adafruit_NeoPixel _pixels;

  int _playerCount;
  std::vector<uint16_t> _playerHues;
  int _maxScore;
  int _targetScore;
  bool _isBlinkOn;

  State _lastState;
  uint16_t _prospectiveFirstHue;
  bool _hasProspectiveFirstHue;

  void illuminate_row(int row, uint16_t hue, float ratio, uint8_t brightness = 255);
  int get_pixel_index(int row, int col);
  int getRemainderBrightness(float remainder, int fullBrightness);

  PlayerRows getRowMapping(int totalPlayers, int playerIdx);
  bool shouldRefresh(const State& newState);
  void renderPlayerRows(PlayerRows rows, uint16_t hue, float ratio, uint8_t brightness);
  uint16_t getPlayerHue(int playerIdx);
};

#endif