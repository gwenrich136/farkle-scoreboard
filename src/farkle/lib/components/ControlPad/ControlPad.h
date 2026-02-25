#ifndef ControlPad_h
#define ControlPad_h

#include <Arduino.h>
#include "Input.h"

// Constants for pins and timing
#define ADC_PIN A2
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define BANK_PIN 6
#define FARKLE_PIN 8

#define ADC_STABILITY_THRESHOLD_MS 50
#define DEBOUNCE_DELAY 50

class ControlPad {
public:
  ControlPad();

  GameInput read();

  // Changed to non-static to access instance members
  void handleInterrupt();

private:
  ButtonAction _lastAction;

  // Encoder state
  volatile int _encoderDelta;

  // ADC state
  int _lastAdcValue;
  unsigned long _adcStableStartTime;
  ButtonAction _currentAdcAction;

  // Digital state
  unsigned long _lastDebounceTime[20]; // Simple array for debouncing digital pins (using pin number as index)
  int _buttonState[20];
  int _lastButtonState[20];

  ButtonAction checkAdc();
  ButtonAction checkDigital();
  ButtonAction mapAdcToAction(int val); // Making this private helper method? Or global static helper?
};

#endif
