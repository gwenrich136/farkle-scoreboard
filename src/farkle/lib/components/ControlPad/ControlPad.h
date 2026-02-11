#ifndef ControlPad_h
#define ControlPad_h

#include <Arduino.h>
#include "ButtonActions.h"

#define MAX_PINS 17 // Max pin number + 1 to support up to pin 16
#define DEBOUNCE_DELAY 50 // ms

class ControlPad {
public:
  ControlPad();
  void addButton(int pin, ButtonAction buttonAction);
  ButtonAction read();

private:
  ButtonAction _buttonMap[MAX_PINS];
  ButtonAction _lastAction;
  int _activePins[MAX_PINS];
  int _activePinCount;

  // Debouncing state
  unsigned long _lastDebounceTime[MAX_PINS];
  int _lastButtonState[MAX_PINS];
  int _buttonState[MAX_PINS];
};

#endif