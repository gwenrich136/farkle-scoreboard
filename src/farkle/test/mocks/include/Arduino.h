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

// Mock Arduino functions
unsigned long millis();
void delay(unsigned long ms);

#endif // ARDUINO_H
