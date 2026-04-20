#include "Arduino.h"
#include <cstdlib>
#include <map>

static unsigned long mocked_millis = 0;
static std::map<int, int> mockPinModes;
static std::map<int, int> mockPinStates;
static std::map<int, int> mockAnalogStates;
static std::map<int, void (*)(void)> mockInterrupts;

unsigned long lastRandomSeed = 0;

void randomSeed(unsigned long seed) {
    lastRandomSeed = seed;
    srand(seed);
}

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

void digitalWrite(int pin, int val) {
    mockPinStates[pin] = val;
}

int digitalRead(int pin) {
    if (mockPinStates.find(pin) == mockPinStates.end()) {
        return HIGH; // Default to HIGH (like INPUT_PULLUP unpressed)
    }
    return mockPinStates[pin];
}

int analogRead(int pin) {
    if (mockAnalogStates.find(pin) == mockAnalogStates.end()) {
        return 0; // Default 0
    }
    return mockAnalogStates[pin];
}

int digitalPinToInterrupt(int pin) {
    return pin; // Simplified mock mapping
}

void attachInterrupt(int interrupt, void (*userFunc)(void), int mode) {
    mockInterrupts[interrupt] = userFunc;
}

void interrupts() {
    // No-op for mock
}

void noInterrupts() {
    // No-op for mock
}

void setMockPinState(int pin, int state) {
    mockPinStates[pin] = state;
}

void setMockAnalogPin(int pin, int val) {
    mockAnalogStates[pin] = val;
}

int getMockPinMode(int pin) {
    if (mockPinModes.find(pin) == mockPinModes.end()) {
        return -1; // -1 for undefined/default
    }
    return mockPinModes[pin];
}

void triggerInterrupt(int pin) {
    if (mockInterrupts.find(pin) != mockInterrupts.end()) {
        mockInterrupts[pin]();
    }
}

void resetMockPins() {
    mockPinModes.clear();
    mockPinStates.clear();
    mockAnalogStates.clear();
    mockInterrupts.clear();
    // mocked_millis is intentionally preserved to simulate continuous time if needed,
    // or tests can manually reset it if they had access, but usually it's fine.
}

long random(long max) {
    if (max <= 0) return 0;
    return rand() % max;
}

long random(long min, long max) {
    if (min >= max) return min;
    return min + (rand() % (max - min));
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    if ((in_max - in_min) == 0) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
