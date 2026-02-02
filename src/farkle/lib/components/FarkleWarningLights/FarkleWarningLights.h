#ifndef FarkleWarningLights_h
#define FarkleWarningLights_h

#include "Arduino.h"

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
    unsigned long _lastToggleTime = 0;
    bool _yellowIsOn = false;
    const int ALTERNATE_INTERVAL = 250; // ms
};

#endif
