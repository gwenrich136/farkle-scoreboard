#include "ScoreDisplay.h"
#include <Arduino.h> // Include Arduino.h for Serial.println

#define NUM_DEVICES 3
#define NUM_DIGITS_PER_DISPLAY 5
#define TEN_THOUSAND 10000

#define SCORE_BLINK_LOW 2
#define SCORE_BLINK_HIGH 12
#define SCORE_DEFAULT_INTENSITY 8

ScoreDisplay::ScoreDisplay(int dataPin, int clkPin, int csPin)
  : _lc(dataPin, clkPin, csPin, NUM_DEVICES)
{
  for (int i = 0; i < 3; i++) {
    _deviceMap[i] = -1;
  }
}

void ScoreDisplay::begin() {
  // Set up the MAX7219 devices
  for (int i = 0; i < NUM_DEVICES; i++) {
    _lc.shutdown(i, false); // Wake up display
    _lc.setIntensity(i, SCORE_DEFAULT_INTENSITY); // Set brightness (0-15)
    _lc.clearDisplay(i);    // Clear display
  }
  for (int i = 0; i < 3; i++) {
    _states[i].isCleared = true;
    _states[i].number = -1;
    _states[i].blink = false;
    _states[i].lastIntensity = -1;
  }
}

void ScoreDisplay::addDisplay(DisplayType type, int deviceIndex) {
  _deviceMap[static_cast<int>(type)] = deviceIndex;
}

void ScoreDisplay::clear(DisplayType type) {
  int typeIdx = static_cast<int>(type);
  if (_states[typeIdx].isCleared) return;

  int deviceIndex = _deviceMap[typeIdx];
  if (deviceIndex == -1) return;
  _lc.clearDisplay(deviceIndex);

  _states[typeIdx].isCleared = true;
  _states[typeIdx].number = -1;
  _states[typeIdx].blink = false;
  _states[typeIdx].lastIntensity = -1;
}

void ScoreDisplay::print_number(int number, DisplayType type, bool blink)
{
  int typeIdx = static_cast<int>(type);
  int deviceIndex = _deviceMap[typeIdx];
  if (deviceIndex == -1) return;

  int numberToDisplay = number;
  if (numberToDisplay > 99999) {
    numberToDisplay = 99999;
  }

  int targetIntensity = blink ?
    ((millis() / 500) % 2 == 0 ? SCORE_BLINK_LOW : SCORE_BLINK_HIGH) :
    SCORE_DEFAULT_INTENSITY;

  bool numberChanged = _states[typeIdx].isCleared || _states[typeIdx].number != numberToDisplay;
  bool intensityChanged = _states[typeIdx].lastIntensity != targetIntensity;
  bool blinkModeChanged = _states[typeIdx].blink != blink;

  if (!numberChanged && !intensityChanged && !blinkModeChanged) {
    return;
  }

  if (intensityChanged || blinkModeChanged) {
    _lc.setIntensity(deviceIndex, targetIntensity);
    _states[typeIdx].lastIntensity = targetIntensity;
    _states[typeIdx].blink = blink;
  }

  if (numberChanged) {
    char digits[12];
    int len = 0;

    int tempNumber = numberToDisplay;
    if (tempNumber == 0) {
      digits[len++] = '0';
    } else {
      bool negative = tempNumber < 0;
      if (negative) tempNumber = -tempNumber;

      while (tempNumber > 0) {
        digits[len++] = (tempNumber % 10) + '0';
        tempNumber /= 10;
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

    _states[typeIdx].number = numberToDisplay;
    _states[typeIdx].isCleared = false;
  }
}
