#ifndef ScoreDisplay_h
#define ScoreDisplay_h

#include <Arduino.h>
#include <LedControl.h>

class ScoreDisplay {
public:
  // dataPin, clkPin, csPin
  ScoreDisplay(int dataPin, int clkPin, int csPin);
  void print_number(int number, int device_index);

private:
  LedControl _lc;
};

#endif