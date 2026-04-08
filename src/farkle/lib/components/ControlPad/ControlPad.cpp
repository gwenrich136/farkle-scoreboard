#include "ControlPad.h"
#include <stdlib.h> // for abs

// Singleton instance for Interrupt Service Routine
static ControlPad* instance = nullptr;

// Interrupt handler wrapper
static void encoderInterruptHandler() {
  if (instance) {
    instance->handleInterrupt();
  }
}

ControlPad::ControlPad() {
  instance = this;
  // State initialization only, no hardware calls here!
  _lastAction = ButtonAction::NONE;
  _encoderDelta = 0;
  _old_AB = 0x03; // Default HIGH/HIGH
  _lastAnalogValue = -1;
  _analogStableStartTime = 0;
  _currentAnalogAction = ButtonAction::NONE;

  for (int i = 0; i < 20; i++) {
      _lastDebounceTime[i] = 0;
      _buttonState[i] = HIGH;
      _lastButtonState[i] = HIGH;
  }
}

void ControlPad::begin() {
  initializeHardware();
}

void ControlPad::initializeHardware() {
  pinMode(BANK_PIN, INPUT_PULLUP);
  pinMode(FARKLE_PIN, INPUT_PULLUP);
  pinMode(SELECT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ANALOG_INPUT_PIN, INPUT);
  pinMode(CURRENT_PLAYER_TOGGLE_PIN, INPUT_PULLUP);

  // Attach Interrupts
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderInterruptHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), encoderInterruptHandler, CHANGE);
}

void ControlPad::handleInterrupt() {
  static const int8_t KNOB_DIR[] = {
    0, -1,  1,  0,
    1,  0,  0, -1,
   -1,  0,  0,  1,
    0,  1, -1,  0
  };

  _old_AB <<= 2;
  _old_AB |= ( (digitalRead(ENCODER_PIN_A) << 1) | digitalRead(ENCODER_PIN_B) );
  _encoderDelta += KNOB_DIR[_old_AB & 0x0F];
}

ButtonAction ControlPad::mapAnalogValueToAction(int val) {
    // 0 (CLEAR), 93 (+50), 328 (+100), 512 (+500)
    if (val < 46) return ButtonAction::CLEAR;
    if (val < 210) return ButtonAction::PLUS_50;
    if (val < 420) return ButtonAction::PLUS_100;
    if (val < 700) return ButtonAction::PLUS_500;
    return ButtonAction::NONE;
}

ButtonAction ControlPad::checkAnalogInput() {
    int val = analogRead(ANALOG_INPUT_PIN);

    // Stability check (tolerance +/- 5)
    if (abs(val - _lastAnalogValue) > 5) {
        _lastAnalogValue = val;
        _analogStableStartTime = millis();
        return ButtonAction::NONE; // Unstable
    }

    // Stable
    if (millis() - _analogStableStartTime > ANALOG_STABILITY_THRESHOLD_MS) {
        return mapAnalogValueToAction(val);
    }

    return ButtonAction::NONE;
}

ButtonAction ControlPad::checkDigitalInput() {
    // Check BANK, FARKLE, and SELECT
    int pins[] = {BANK_PIN, FARKLE_PIN, SELECT_PIN};
    ButtonAction actions[] = {ButtonAction::BANK, ButtonAction::FARKLE, ButtonAction::SELECT};
    ButtonAction result = ButtonAction::NONE;

    for (int i = 0; i < 3; i++) {
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

        // Collect the first pressed button detected in the loop
        if (_buttonState[pin] == LOW && result == ButtonAction::NONE) {
            result = actions[i];
        }
    }
    return result;
}

GameInput ControlPad::read() {
    GameInput input;
    input.action = ButtonAction::NONE;
    input.rotationDelta = 0;

    if (digitalRead(CURRENT_PLAYER_TOGGLE_PIN) == LOW) {
        input.scoreDisplayMode = ScoreDisplayMode::PENDING;
    } else {
        input.scoreDisplayMode = ScoreDisplayMode::BANKED;
    }

    // 1. Check Digital Priority
    input.action = checkDigitalInput();

    // 2. Check Analog Input (if no digital action)
    if (input.action == ButtonAction::NONE) {
        input.action = checkAnalogInput();
    }

    // 3. Process Action or Rotation
    if (input.action != ButtonAction::NONE) {
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
    } else {
        // No action, process rotation
        noInterrupts();
        int rawDelta = _encoderDelta;
        _encoderDelta = 0;
        interrupts();

        // Standard encoders (KY-040) have 4 pulses per physical click (detent)
        input.rotationDelta = rawDelta / 4;
        
        // If we didn't reach a full click, we need to preserve the remainder
        // to prevent "dead zones" where slow turning does nothing.
        static int fractionalPulses = 0;
        fractionalPulses += (rawDelta % 4);
        
        if (abs(fractionalPulses) >= 4) {
            input.rotationDelta += (fractionalPulses / 4);
            fractionalPulses %= 4;
        }

        // Reset last action if button released (meaning no action detected this frame)
        _lastAction = ButtonAction::NONE;
    }

    return input;
}
