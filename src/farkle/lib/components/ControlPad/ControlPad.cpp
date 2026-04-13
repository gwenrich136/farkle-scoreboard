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

  _lastBusState = 0;
  _stableBusState = 0;
  _lastBusDebounceTime = 0;

  _selectButtonState = HIGH;
  _lastSelectButtonState = HIGH;
  _lastSelectDebounceTime = 0;
}

void ControlPad::begin() {
  initializeHardware();
}

void ControlPad::initializeHardware() {
  pinMode(CONTROL_PAD_BUS_0_PIN, INPUT_PULLUP);
  pinMode(CONTROL_PAD_BUS_1_PIN, INPUT_PULLUP);
  pinMode(CONTROL_PAD_BUS_2_PIN, INPUT_PULLUP);
  pinMode(SELECT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
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

ButtonAction ControlPad::mapBusStateToAction(uint8_t state) {
    switch (state) {
        case 1: return ButtonAction::CLEAR;
        case 2: return ButtonAction::FARKLE;
        case 3: return ButtonAction::PLUS_500;
        case 4: return ButtonAction::BANK;
        case 5: return ButtonAction::UNDO;
        case 6: return ButtonAction::PLUS_100;
        case 7: return ButtonAction::PLUS_50;
        case 0:
        default:
            return ButtonAction::NONE;
    }
}

ButtonAction ControlPad::checkBusInput() {
    uint8_t bit0 = digitalRead(CONTROL_PAD_BUS_0_PIN) == LOW ? 1 : 0;
    uint8_t bit1 = digitalRead(CONTROL_PAD_BUS_1_PIN) == LOW ? 1 : 0;
    uint8_t bit2 = digitalRead(CONTROL_PAD_BUS_2_PIN) == LOW ? 1 : 0;

    uint8_t currentBusState = (bit2 << 2) | (bit1 << 1) | bit0;

    if (currentBusState != _lastBusState) {
        _lastBusDebounceTime = millis();
    }

    if ((millis() - _lastBusDebounceTime) > DEBOUNCE_DELAY) {
        if (currentBusState != _stableBusState) {
            _stableBusState = currentBusState;
        }
    }

    _lastBusState = currentBusState;

    return mapBusStateToAction(_stableBusState);
}

ButtonAction ControlPad::checkSelectInput() {
    int reading = digitalRead(SELECT_PIN);

    if (reading != _lastSelectButtonState) {
        _lastSelectDebounceTime = millis();
    }

    if ((millis() - _lastSelectDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != _selectButtonState) {
            _selectButtonState = reading;
        }
    }

    _lastSelectButtonState = reading;

    if (_selectButtonState == LOW) {
        return ButtonAction::SELECT;
    }

    return ButtonAction::NONE;
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

    // 1. Check Select Button Priority
    input.action = checkSelectInput();

    // 2. Check Bus Input (if no select action)
    if (input.action == ButtonAction::NONE) {
        input.action = checkBusInput();
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
