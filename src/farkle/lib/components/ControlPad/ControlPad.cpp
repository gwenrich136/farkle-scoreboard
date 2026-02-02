#include "ControlPad.h"
#include "ButtonActions.h"

ControlPad::ControlPad() : _lastAction(ButtonAction::NONE) {
  // Initialize all pins to NONE
  for (int i = 0; i < MAX_PINS; i++) {
    _buttonMap[i] = ButtonAction::NONE;
  }
}

void ControlPad::addButton(int pin, ButtonAction buttonAction) {
  if (pin < MAX_PINS) {
    _buttonMap[pin] = buttonAction;
    pinMode(pin, INPUT_PULLUP); // Configure pin as input with pull-up resistor
  }
}

ButtonAction ControlPad::read() {
  int pressedCount = 0;
  ButtonAction pressedAction = ButtonAction::NONE;

  for (int i = 0; i < MAX_PINS; i++) {
    // If a button action is assigned to this pin
    if (_buttonMap[i] != ButtonAction::NONE) {
      if (digitalRead(i) == LOW) { // Button is pressed (LOW due to INPUT_PULLUP)
        pressedCount++;
        pressedAction = _buttonMap[i];
      }
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