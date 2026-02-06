#ifndef ScoreDisplay_h
#define ScoreDisplay_h

#include <Arduino.h>
#include <LedControl.h>

class ScoreDisplay {
public:
  enum DisplayType { AT_RISK_SCORE, CURRENT_PLAYER_SCORE, COMPETITION_SCORE };

  // dataPin, clkPin, csPin
  ScoreDisplay(int dataPin, int clkPin, int csPin);
  void begin();
  void addDisplay(DisplayType type, int deviceIndex);
  void print_number(int number, DisplayType type, bool blink = false);
  void clear(DisplayType type);

  struct State {
    int number = -1;
    bool blink = false;
    bool isCleared = true;
    int lastIntensity = -1;

    bool operator==(const State& other) const {
      return number == other.number &&
             blink == other.blink &&
             isCleared == other.isCleared &&
             lastIntensity == other.lastIntensity;
    }
    bool operator!=(const State& other) const { return !(*this == other); }
  };

private:
  LedControl _lc;
  int _deviceMap[3]; // Map DisplayType to deviceIndex
  State _states[3];
};

#endif
