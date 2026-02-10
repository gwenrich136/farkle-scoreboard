#include "ControlPad.h"
#include "ButtonActions.h"
#include <cassert>

ControlPad::ControlPad() : _lastAction(ButtonAction::NONE), _activePinCount(0) {
  // Initialize all pins to NONE
  for (int i = 0; i < MAX_PINS; i++) {
    _buttonMap[i] = ButtonAction::NONE;
  }
}

void ControlPad::addButton(int pin, ButtonAction buttonAction) {
  if (pin < 0 || pin >= MAX_PINS) {
    return;
  }

  if (buttonAction == ButtonAction::NONE) {
    Serial.println("Error: Cannot map button to NONE");
#ifndef UNIT_TEST
    assert(false);
#endif
    return;
  }

  if (_buttonMap[pin] != ButtonAction::NONE) {
    Serial.println("Error: Pin already mapped");
#ifndef UNIT_TEST
    assert(false);
#endif
    return;
  }

  _buttonMap[pin] = buttonAction;
  _activePins[_activePinCount++] = pin;
  pinMode(pin, INPUT_PULLUP); // Configure pin as input with pull-up resistor
}

ButtonAction ControlPad::read() {
  int pressedCount = 0;
  ButtonAction pressedAction = ButtonAction::NONE;

  for (int i = 0; i < _activePinCount; i++) {
    int pin = _activePins[i];
    // If a button action is assigned to this pin
    if (_buttonMap[pin] != ButtonAction::NONE) {
      if (digitalRead(pin) == LOW) { // Button is pressed (LOW due to INPUT_PULLUP)
        pressedCount++;
        pressedAction = _buttonMap[pin];
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