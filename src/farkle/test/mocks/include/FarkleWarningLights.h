#ifndef MOCK_FARKLE_WARNING_LIGHTS_H
#define MOCK_FARKLE_WARNING_LIGHTS_H

class FarkleWarningLights {
public:
    int captured_state;

    FarkleWarningLights(int yellowPin, int redPin);
    void begin();
    void farkle_state(int state);
};

#endif // MOCK_FARKLE_WARNING_LIGHTS_H
