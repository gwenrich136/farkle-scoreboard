#include "ControlPad.h"
#include "ButtonActions.h"

ControlPad::ControlPad() : _lastAction(ButtonAction::NONE), _activePinCount(0) {
  // Initialize all pins to NONE
  for (int i = 0; i < MAX_PINS; i++) {
    _buttonMap[i] = ButtonAction::NONE;
    _lastDebounceTime[i] = 0;
    _lastButtonState[i] = HIGH; // Pull-up means default state is HIGH
    _buttonState[i] = HIGH;
  }
}

void ControlPad::addButton(int pin, ButtonAction buttonAction) {
  if (pin < 0 || pin >= MAX_PINS) {
    return;
  }

  if (buttonAction == ButtonAction::NONE) {
    Serial.println("Error: Cannot map button to NONE");
    return;
  }

  if (_buttonMap[pin] != ButtonAction::NONE) {
    Serial.println("Error: Pin already mapped");
    return;
  }

  _buttonMap[pin] = buttonAction;
  _activePins[_activePinCount++] = pin;
  _lastDebounceTime[pin] = 0;
  _lastButtonState[pin] = HIGH;
  _buttonState[pin] = HIGH;
  pinMode(pin, INPUT_PULLUP); // Configure pin as input with pull-up resistor
}

ButtonAction ControlPad::read() {
  int pressedCount = 0;
  ButtonAction pressedAction = ButtonAction::NONE;

  unsigned long now = millis();

  for (int i = 0; i < _activePinCount; i++) {
    int pin = _activePins[i];

    int reading = digitalRead(pin);

    if (reading != _lastButtonState[pin]) {
      _lastDebounceTime[pin] = now;
    }

    if ((now - _lastDebounceTime[pin]) > DEBOUNCE_DELAY) {
      if (reading != _buttonState[pin]) {
        _buttonState[pin] = reading;
      }
    }

    _lastButtonState[pin] = reading;

    // Use the debounced state
    if (_buttonState[pin] == LOW) { // Button is pressed (LOW due to INPUT_PULLUP)
      pressedCount++;
      pressedAction = _buttonMap[pin];
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