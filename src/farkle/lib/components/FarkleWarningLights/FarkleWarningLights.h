#ifndef FarkleWarningLights_h
#define FarkleWarningLights_h

#include "Arduino.h"

class FarkleWarningLights
{
  public:
    FarkleWarningLights(int yellowPin, int redPin);
    void farkle_state(int state);
  private:
    int _yellowPin;
    int _redPin;
};

#endif
