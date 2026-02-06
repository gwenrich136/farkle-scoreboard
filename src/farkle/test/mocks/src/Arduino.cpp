#include "Arduino.h"
#include <cstdlib>
#include <map>

static unsigned long mocked_millis = 0;
static std::map<int, int> mockPinModes;
static std::map<int, int> mockPinStates;

unsigned long millis() {
    return mocked_millis;
}

void delay(unsigned long ms) {
    mocked_millis += ms;
}

void advance_millis(unsigned long ms) {
    mocked_millis += ms;
}

void pinMode(int pin, int mode) {
    mockPinModes[pin] = mode;
}

int digitalRead(int pin) {
    if (mockPinStates.find(pin) == mockPinStates.end()) {
        return HIGH; // Default to HIGH (like INPUT_PULLUP unpressed)
    }
    return mockPinStates[pin];
}

void setMockPinState(int pin, int state) {
    mockPinStates[pin] = state;
}

int getMockPinMode(int pin) {
    if (mockPinModes.find(pin) == mockPinModes.end()) {
        return -1; // -1 for undefined/default
    }
    return mockPinModes[pin];
}

void resetMockPins() {
    mockPinModes.clear();
    mockPinStates.clear();
}

long random(long max) {
    if (max <= 0) return 0;
    return rand() % max;
}

long random(long min, long max) {
    if (min >= max) return min;
    return min + (rand() % (max - min));
}
