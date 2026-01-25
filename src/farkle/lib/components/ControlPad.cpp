#include "ControlPad.h"

ControlPad::ControlPad() {
  // Constructor is empty as per requirement
}

void ControlPad::addButton(int pin, ButtonAction buttonAction) {
  _buttonMap[pin] = buttonAction;
  pinMode(pin, INPUT_PULLUP); // Configure pin as input with pull-up resistor
}

ControlPad::ButtonAction ControlPad::read() {
  int pressedCount = 0;
  ButtonAction pressedAction = ButtonAction::NONE;

  for (auto const& [pin, action] : _buttonMap) {
    if (digitalRead(pin) == LOW) { // Button is pressed (LOW due to INPUT_PULLUP)
      pressedCount++;
      pressedAction = action;
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