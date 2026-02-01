#ifndef ControlPad_h
#define ControlPad_h

#include <Arduino.h>
#include <map> // Required for std::map
#include "ButtonActions.h"

class ControlPad {
public:
  ControlPad();
  void addButton(int pin, ButtonAction buttonAction);
  ButtonAction read();

private:
  std::map<int, ButtonAction> _buttonMap;
  ButtonAction _lastAction;
};

#endif