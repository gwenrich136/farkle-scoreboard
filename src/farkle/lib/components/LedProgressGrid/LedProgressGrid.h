#ifndef LedProgressGrid_h
#define LedProgressGrid_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <vector> // Required for std::vector

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
    std::vector<int> scores;
    int currentPlayerIndex = -1;
    int atRiskScore = -1;
    bool isBlinkOn = false;
    bool isPlayerPending = false;
    int playerCount = 0;
    bool isDirty = true;

    bool operator==(const State& other) const {
      return !isDirty && !other.isDirty &&
             mode == other.mode &&
             scores == other.scores &&
             currentPlayerIndex == other.currentPlayerIndex &&
             atRiskScore == other.atRiskScore &&
             isBlinkOn == other.isBlinkOn &&
             isPlayerPending == other.isPlayerPending &&
             playerCount == other.playerCount;
    }
    bool operator!=(const State& other) const { return !(*this == other); }
  };

  LedProgressGrid(uint8_t pin);
  void begin();
  int addPlayer();
  void reset();
  void clear();
  void update(const std::vector<int>& scores, int currentPlayerIndex, int atRiskScore);
  void displayPlayersPregame(bool isPlayerPending);

private:
  Adafruit_NeoPixel _pixels;

  int _playerCount;
  std::vector<uint16_t> _playerHues;
  int _maxScore;
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
  uint16_t getPlayerHue(int playerIdx, int totalPlayers);
};

#endif