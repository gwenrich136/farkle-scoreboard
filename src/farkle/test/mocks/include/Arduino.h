#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstdint>
#include "Serial.h"

// Define common Arduino types
using byte = uint8_t;

// Define mock pin constants
// Using high values to avoid conflict with digital pins 0-13
const int A0 = 100;
const int A1 = 101;
const int A2 = 102;
const int A3 = 103;
const int A4 = 104;
const int A5 = 105;
const int D7 = 7;
const int D8 = 8;

// Pin modes
const int INPUT = 0;
const int OUTPUT = 1;
const int INPUT_PULLUP = 2;

// Pin levels
const int LOW = 0;
const int HIGH = 1;

// Interrupt modes
const int RISING = 3;
const int FALLING = 4;
const int CHANGE = 5;

// Mock Arduino functions
unsigned long millis();
void delay(unsigned long ms);
void advance_millis(unsigned long ms);

void pinMode(int pin, int mode);
void digitalWrite(int pin, int val);
int digitalRead(int pin);
int analogRead(int pin);

int digitalPinToInterrupt(int pin);
void attachInterrupt(int interrupt, void (*userFunc)(void), int mode);
void interrupts();
void noInterrupts();

// Test helpers
void setMockPinState(int pin, int state);
void setMockAnalogPin(int pin, int val);
int getMockPinMode(int pin);
void triggerInterrupt(int pin);
void resetMockPins();

void randomSeed(unsigned long seed);
extern unsigned long lastRandomSeed;

long random(long max);
long random(long min, long max);
long map(long x, long in_min, long in_max, long out_min, long out_max);

template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

#include <string>

class String : public std::string {
public:
    String() : std::string() {}
    String(const char* s) : std::string(s) {}
    String(const std::string& s) : std::string(s) {}

    int toInt() const {
        try {
            return std::stoi(*this);
        } catch (...) {
            return 0;
        }
    }
};

#endif // ARDUINO_H
