#ifndef ScoreDisplay_h
#define ScoreDisplay_h

#include <Arduino.h>
#include <LedControl.h>

class ScoreDisplay {
public:
  enum class DisplayType { AT_RISK_SCORE, CURRENT_PLAYER_SCORE, COMPETITION_SCORE };

  // dataPin, clkPin, csPin
  ScoreDisplay(int dataPin, int clkPin, int csPin);
  void begin();
  void addDisplay(DisplayType type, int deviceIndex);
  void print_number(int number, DisplayType type, bool blink = false);
  void clear(DisplayType type);

private:
  LedControl _lc;
  int _deviceMap[3]; // Map DisplayType to deviceIndex
};

#endif
