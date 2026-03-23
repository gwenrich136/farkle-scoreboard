#ifndef ControlPad_h
#define ControlPad_h

#include <Arduino.h>
#include "Input.h"

// Constants for pins and timing
#define ANALOG_INPUT_PIN A2
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define BANK_PIN 5
#define FARKLE_PIN 6
#define SELECT_PIN 4

#define ANALOG_STABILITY_THRESHOLD_MS 50
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

  // Analog Input state
  int _lastAnalogValue;
  unsigned long _analogStableStartTime;
  ButtonAction _currentAnalogAction;

  // Digital state
  unsigned long _lastDebounceTime[20]; // Simple array for debouncing digital pins (using pin number as index)
  int _buttonState[20];
  int _lastButtonState[20];

  ButtonAction checkAnalogInput();
  ButtonAction checkDigitalInput();
  ButtonAction mapAnalogValueToAction(int val);
  void initializeHardware();
};

#endif
