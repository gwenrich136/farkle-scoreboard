#ifndef ControlPad_h
#define ControlPad_h

#include <Arduino.h>
#include "ButtonActions.h"

#define MAX_PINS 17 // Max pin number + 1 to support up to pin 16

class ControlPad {
public:
  ControlPad();
  void addButton(int pin, ButtonAction buttonAction);
  ButtonAction read();

private:
  ButtonAction _buttonMap[MAX_PINS];
  ButtonAction _lastAction;
};

#endif