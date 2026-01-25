#ifndef ControlPad_h
#define ControlPad_h

#include <Arduino.h>
#include <map> // Required for std::map

class ControlPad {
public:
  enum ButtonAction {
    DOWN_50,
    FARKLE,
    BANK,
    CLEAR,
    NONE
  };

  ControlPad();
  void addButton(int pin, ButtonAction buttonAction);
  ButtonAction read();

private:
  std::map<int, ButtonAction> _buttonMap;
  ButtonAction _lastAction;
};

#endif