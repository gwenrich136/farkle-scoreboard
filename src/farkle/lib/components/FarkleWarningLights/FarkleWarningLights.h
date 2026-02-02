#ifndef FarkleWarningLights_h
#define FarkleWarningLights_h

#include "Arduino.h"

#define ALTERNATE_INTERVAL 250 // ms

class FarkleWarningLights
{
  public:
    FarkleWarningLights(int yellowPin, int redPin);
    void begin();
    void farkle_state(int state);
    void alternate();
  private:
    int _yellowPin;
    int _redPin;
};

#endif
