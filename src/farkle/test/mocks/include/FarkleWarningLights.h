#ifndef MOCK_FARKLE_WARNING_LIGHTS_H
#define MOCK_FARKLE_WARNING_LIGHTS_H

#include <vector>

class FarkleWarningLights {
public:
    int captured_state;
    std::vector<int> captured_farkleCounts;
    int captured_playerCount;
    int captured_currentPlayerIndex;
    bool captured_isBlinking;

    FarkleWarningLights(int pin);
    void begin();
    void farkle_state(int state);
    void update(const int* farkleCounts, int playerCount, int currentPlayerIndex, bool isBlinking);
    void alternate(int currentPlayerIndex, int playerCount);
};

#endif // MOCK_FARKLE_WARNING_LIGHTS_H
