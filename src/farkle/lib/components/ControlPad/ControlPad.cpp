#include "ControlPad.h"
#include "ButtonActions.h"

ControlPad::ControlPad() : _buttonCount(0), _lastAction(ButtonAction::NONE) {
  // Initialize _buttonCount to 0 and lastAction to NONE
}

void ControlPad::addButton(int pin, ButtonAction buttonAction) {
  if (_buttonCount < 16) {
    _buttonMap[_buttonCount] = {pin, buttonAction};
    _buttonCount++;
    pinMode(pin, INPUT_PULLUP); // Configure pin as input with pull-up resistor
  }
}

ButtonAction ControlPad::read() {
  int pressedCount = 0;
  ButtonAction pressedAction = ButtonAction::NONE;

  for (int i = 0; i < _buttonCount; i++) {
    if (digitalRead(_buttonMap[i].pin) == LOW) { // Button is pressed (LOW due to INPUT_PULLUP)
      pressedCount++;
      pressedAction = _buttonMap[i].action;
    }
  }

  if (pressedCount > 1) {
    return ButtonAction::NONE;
  }
  if (pressedAction == _lastAction) {
    // To ensure holding a button counts as one press
    return ButtonAction::NONE; 
  }
  
  _lastAction = pressedAction;
  return pressedAction;
}