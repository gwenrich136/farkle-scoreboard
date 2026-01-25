#include "FarkleWarningLights.h"

FarkleWarningLights::FarkleWarningLights(int yellowPin, int redPin)
{
  _yellowPin = yellowPin;
  _redPin = redPin;
  pinMode(_yellowPin, OUTPUT);
  pinMode(_redPin, OUTPUT);
}

void FarkleWarningLights::farkle_state(int state) {
  if (state < 1) {
    digitalWrite(_yellowPin, LOW);
  } else {
    digitalWrite(_yellowPin, HIGH);
  }
  if (state < 2) {
    digitalWrite(_redPin, LOW);
  } else {
    digitalWrite(_redPin, HIGH);
  }
}