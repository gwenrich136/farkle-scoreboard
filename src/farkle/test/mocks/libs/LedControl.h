#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <stdint.h>
#include <map>
#include <string> // Required for current ScoreDisplay implementation

// Global state for verification
// Map<DeviceIndex, Map<DigitIndex, Character>>
extern std::map<int, std::map<int, char>> mockLedState;
// Map<DeviceIndex, Intensity>
extern std::map<int, int> mockLedIntensity;

class LedControl {
public:
    LedControl(int dataPin, int clkPin, int csPin, int numDevices) {
    }

    void shutdown(int addr, bool b) {}

    void setIntensity(int addr, int intensity) {
        mockLedIntensity[addr] = intensity;
    }

    void clearDisplay(int addr) {
        mockLedState[addr].clear();
    }

    void setChar(int addr, int digit, char value, bool dp) {
        mockLedState[addr][digit] = value;
    }
};

#endif
