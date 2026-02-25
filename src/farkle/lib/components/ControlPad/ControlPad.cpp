#include "ControlPad.h"
#include <stdlib.h> // for abs

// Singleton instance for ISR
static ControlPad* instance = nullptr;

// ISR handler wrapper
static void encoderISR() {
  if (instance) {
    instance->handleInterrupt();
  }
}

ControlPad::ControlPad() {
  instance = this;
  _lastAction = ButtonAction::NONE;
  _encoderDelta = 0;

  _lastAdcValue = -1;
  _adcStableStartTime = 0;
  _currentAdcAction = ButtonAction::NONE;

  for (int i = 0; i < 20; i++) {
      _lastDebounceTime[i] = 0;
      _buttonState[i] = HIGH;
      _lastButtonState[i] = HIGH;
  }

  // Setup Pins
  pinMode(BANK_PIN, INPUT_PULLUP);
  pinMode(FARKLE_PIN, INPUT_PULLUP);
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ADC_PIN, INPUT);

  // Attach Interrupts
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), encoderISR, CHANGE);
}

void ControlPad::handleInterrupt() {
  static int lastA = HIGH;
  static int lastB = HIGH;

  int newA = digitalRead(ENCODER_PIN_A);
  int newB = digitalRead(ENCODER_PIN_B);

  if (newA != lastA || newB != lastB) {
      if (newA != lastA) {
          if (newA == newB) _encoderDelta--;
          else _encoderDelta++;
      } else {
          if (newA != newB) _encoderDelta--;
          else _encoderDelta++;
      }
  }

  lastA = newA;
  lastB = newB;
}

ButtonAction ControlPad::mapAdcToAction(int val) {
    // 0 (CLEAR), 93 (+50), 328 (+100), 512 (+500)
    if (val < 46) return ButtonAction::CLEAR;
    if (val < 210) return ButtonAction::PLUS_50;
    if (val < 420) return ButtonAction::PLUS_100;
    if (val < 700) return ButtonAction::PLUS_500;
    return ButtonAction::NONE;
}

ButtonAction ControlPad::checkAdc() {
    int val = analogRead(ADC_PIN);

    // Stability check (tolerance +/- 5)
    if (abs(val - _lastAdcValue) > 5) {
        _lastAdcValue = val;
        _adcStableStartTime = millis();
        return ButtonAction::NONE; // Unstable
    }

    // Stable
    if (millis() - _adcStableStartTime > ADC_STABILITY_THRESHOLD_MS) {
        return mapAdcToAction(val);
    }

    return ButtonAction::NONE;
}

ButtonAction ControlPad::checkDigital() {
    // Check BANK and FARKLE
    int pins[] = {BANK_PIN, FARKLE_PIN};
    ButtonAction actions[] = {ButtonAction::BANK, ButtonAction::FARKLE};

    for (int i = 0; i < 2; i++) {
        int pin = pins[i];
        int reading = digitalRead(pin);

        if (reading != _lastButtonState[pin]) {
            _lastDebounceTime[pin] = millis();
        }

        if ((millis() - _lastDebounceTime[pin]) > DEBOUNCE_DELAY) {
            if (reading != _buttonState[pin]) {
                _buttonState[pin] = reading;
            }
        }

        _lastButtonState[pin] = reading;

        if (_buttonState[pin] == LOW) {
            return actions[i];
        }
    }
    return ButtonAction::NONE;
}

GameInput ControlPad::read() {
    GameInput input;
    input.action = ButtonAction::NONE;
    input.rotationDelta = 0;

    // 1. Check Digital Priority
    ButtonAction digitalAction = checkDigital();
    if (digitalAction != ButtonAction::NONE) {
        input.action = digitalAction;

        // Suppress rotation and reset delta
        noInterrupts();
        _encoderDelta = 0;
        interrupts();
        input.rotationDelta = 0;

        // Auto-repeat filtering
        if (_lastAction == input.action) {
            input.action = ButtonAction::NONE;
        } else {
            _lastAction = input.action;
        }
        return input;
    }

    // 2. Check ADC
    ButtonAction adcAction = checkAdc();
    if (adcAction != ButtonAction::NONE) {
        input.action = adcAction;

        // Suppress rotation
        noInterrupts();
        _encoderDelta = 0;
        interrupts();
        input.rotationDelta = 0;

        if (_lastAction == input.action) {
            input.action = ButtonAction::NONE;
        } else {
            _lastAction = input.action;
        }
        return input;
    }

    // 3. Encoder
    // If no action, return rotation
    noInterrupts();
    input.rotationDelta = _encoderDelta;
    _encoderDelta = 0;
    interrupts();

    // If just rotation, action is NONE.
    // Reset last action if button released (meaning no action detected this frame)
    if (input.action == ButtonAction::NONE) {
        _lastAction = ButtonAction::NONE;
    }

    return input;
}
