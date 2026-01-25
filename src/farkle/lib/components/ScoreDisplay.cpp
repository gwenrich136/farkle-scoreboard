#include "ScoreDisplay.h"
#include <Arduino.h> // Include Arduino.h for Serial.println

#define NUM_DEVICES 3
#define NUM_DIGITS_PER_DISPLAY 5
#define TEN_THOUSAND 10000

ScoreDisplay::ScoreDisplay(int dataPin, int clkPin, int csPin)
  : _lc(dataPin, clkPin, csPin)
{
  // Set up the MAX7219 devices
  for (int i = 0; i < NUM_DEVICES; i++) {
    _lc.shutdown(i, false); // Wake up display
    _lc.setIntensity(i, 8); // Set brightness (0-15)
    _lc.clearDisplay(i);    // Clear display
  }
}


void ScoreDisplay::print_number(int number, int deviceIndex)
{
  const int numberToDisplay = number % 100000;
  std::string numberStr = std::to_string(numberToDisplay);
  const int emptySlots = NUM_DIGITS_PER_DISPLAY - numberStr.length();
  for (int i = 0; i < emptySlots; ++i) {
    _lc.setChar(deviceIndex, i, ' ', false);
  }
  for (int i = 0; i < numberStr.length(); ++i){
    _lc.setChar(deviceIndex, i + emptySlots, numberStr.at(i), false);
  }
}