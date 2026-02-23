#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstdint>
#include "Serial.h"

// Define common Arduino types
using byte = uint8_t;

// Define mock pin constants
const int A0 = 0;
const int A1 = 1;
const int A2 = 2;

// Pin modes
const int INPUT = 0;
const int OUTPUT = 1;
const int INPUT_PULLUP = 2;

// Pin levels
const int LOW = 0;
const int HIGH = 1;

// Mock Arduino functions
unsigned long millis();
void delay(unsigned long ms);
void advance_millis(unsigned long ms);

void pinMode(int pin, int mode);
int digitalRead(int pin);

// Test helpers
void setMockPinState(int pin, int state);
int getMockPinMode(int pin);
void resetMockPins();

long random(long max);
long random(long min, long max);
long map(long x, long in_min, long in_max, long out_min, long out_max);

template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

#endif // ARDUINO_H
