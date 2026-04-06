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

extern int mockSetIntensityCount;
extern int mockClearDisplayCount;
extern int mockSetCharCount;

class LedControl {
public:
    LedControl(int dataPin, int clkPin, int csPin, int numDevices) {
    }

    void shutdown(int addr, bool b) {}

    void setIntensity(int addr, int intensity) {
        mockSetIntensityCount++;
        mockLedIntensity[addr] = intensity;
    }

    void clearDisplay(int addr) {
        mockClearDisplayCount++;
        mockLedState[addr].clear();
        for (int i = 0; i < 8; i++) {
            mockLedState[addr][i] = ' ';
        }
    }

    void setChar(int addr, int digit, char value, bool dp) {
        mockSetCharCount++;
        mockLedState[addr][digit] = value;
    }
};

#endif
