#ifndef ControlPad_h
#define ControlPad_h

#include <Arduino.h>
#include <map> // Required for std::map

class ControlPad {
public:
  enum ButtonAction {
    DOWN_50, // Either "down" or +50 poitnts
    LEFT_100, // Either "left" or +100 poitnts
    RIGHT_500, // Either "right" or +500 poitnts
    UP_1000, // Either "up" or +1000 poitnts
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