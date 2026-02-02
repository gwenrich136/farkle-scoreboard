#ifndef ControlPad_h
#define ControlPad_h

#include <Arduino.h>
#include "ButtonActions.h"

// Struct to hold a pin-action pair
struct ButtonMapping {
  int pin;
  ButtonAction action;
};

class ControlPad {
public:
  ControlPad();
  void addButton(int pin, ButtonAction buttonAction);
  ButtonAction read();

private:
  ButtonMapping _buttonMap[16];
  int _buttonCount;
  ButtonAction _lastAction;
};

#endif