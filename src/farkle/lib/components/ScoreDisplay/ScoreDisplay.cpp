#include "ScoreDisplay.h"
#include <Arduino.h> // Include Arduino.h for Serial.println

#define NUM_DEVICES 3
#define NUM_DIGITS_PER_DISPLAY 5
#define TEN_THOUSAND 10000

#define SCORE_BLINK_LOW 4
#define SCORE_BLINK_HIGH 10
#define SCORE_DEFAULT_INTENSITY 8

ScoreDisplay::ScoreDisplay(int dataPin, int clkPin, int csPin)
  : _lc(dataPin, clkPin, csPin, NUM_DEVICES)
{
}

void ScoreDisplay::begin() {
  // Set up the MAX7219 devices
  for (int i = 0; i < NUM_DEVICES; i++) {
    _lc.shutdown(i, false); // Wake up display
    _lc.setIntensity(i, SCORE_DEFAULT_INTENSITY); // Set brightness (0-15)
    _lc.clearDisplay(i);    // Clear display
  }
}


void ScoreDisplay::print_number(int number, int deviceIndex, bool blink)
{
  int numberToDisplay = number;
  if (numberToDisplay > 99999) {
    numberToDisplay = 99999;
  }

  if (blink) {
    int intensity = (millis() / 500) % 2 == 0 ? SCORE_BLINK_LOW : SCORE_BLINK_HIGH;
    _lc.setIntensity(deviceIndex, intensity);
  } else {
    _lc.setIntensity(deviceIndex, SCORE_DEFAULT_INTENSITY);
  }

  char digits[12];
  int len = 0;

  if (numberToDisplay == 0) {
    digits[len++] = '0';
  } else {
    bool negative = numberToDisplay < 0;
    if (negative) numberToDisplay = -numberToDisplay;

    while (numberToDisplay > 0) {
      digits[len++] = (numberToDisplay % 10) + '0';
      numberToDisplay /= 10;
    }
    if (negative) digits[len++] = '-';
  }

  const int emptySlots = NUM_DIGITS_PER_DISPLAY - len;
  for (int i = 0; i < emptySlots; ++i) {
    _lc.setChar(deviceIndex, i, ' ', false);
  }
  for (int i = 0; i < len; ++i){
    _lc.setChar(deviceIndex, i + emptySlots, digits[len - 1 - i], false);
  }
}