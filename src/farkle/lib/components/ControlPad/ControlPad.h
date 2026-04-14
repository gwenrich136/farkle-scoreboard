#ifndef ControlPad_h
#define ControlPad_h

#include <Arduino.h>
#include "Input.h"

// Constants for pins and timing
#define CONTROL_PAD_BUS_0_PIN A2
#define CURRENT_PLAYER_TOGGLE_PIN A3
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define CONTROL_PAD_BUS_2_PIN 5
#define CONTROL_PAD_BUS_1_PIN 6
#define SELECT_PIN 4

#define DEBOUNCE_DELAY 50

class ControlPad {
public:
  ControlPad();

  void begin(); // Added for safe hardware initialization
  GameInput read();

  void handleInterrupt();

private:
  ButtonAction _lastAction;

  // Encoder state
  volatile int _encoderDelta;
  uint8_t _old_AB;

  // Bus Input state
  uint8_t _lastBusState;
  uint8_t _stableBusState;
  unsigned long _lastBusDebounceTime;

  // Select Button state
  int _selectButtonState;
  int _lastSelectButtonState;
  unsigned long _lastSelectDebounceTime;

  ButtonAction checkBusInput();
  ButtonAction checkSelectInput();
  ButtonAction mapBusStateToAction(uint8_t state);
  void initializeHardware();
  ScoreDisplayMode readScoreDisplayMode();
  int processEncoderRotation();
  ButtonAction applyAutoRepeatFilter(ButtonAction currentAction);
};

#endif
