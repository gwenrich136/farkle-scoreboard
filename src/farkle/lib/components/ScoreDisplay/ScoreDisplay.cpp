#include "ScoreDisplay.h"
#include <Arduino.h> // Include Arduino.h for Serial.println

#define NUM_DEVICES 3
#define NUM_DIGITS_PER_DISPLAY 5

#define SCORE_BLINK_LOW 2
#define SCORE_BLINK_HIGH 12
#define SCORE_DEFAULT_INTENSITY 8

ScoreDisplay::ScoreDisplay(int dataPin, int clkPin, int csPin)
  : _lc(dataPin, clkPin, csPin, NUM_DEVICES)
{
  for (int i = 0; i < NUM_DISPLAY_TYPES; i++) {
    _deviceMap[i] = -1;
  }
}

bool ScoreDisplay::isValidType(DisplayType type) {
  int t = static_cast<int>(type);
  if (t < 0 || t >= NUM_DISPLAY_TYPES) {
    Serial.println("Assertion failure: Invalid DisplayType");
    return false;
  }
  return true;
}

void ScoreDisplay::begin() {
  // Set up the MAX7219 devices
  for (int i = 0; i < NUM_DEVICES; i++) {
    _lc.shutdown(i, false); // Wake up display
    _lc.setIntensity(i, SCORE_DEFAULT_INTENSITY); // Set brightness (0-15)
    _lc.clearDisplay(i);    // Clear display
  }

  setState(DisplayType::AT_RISK_SCORE, -1, false, true, -1);
  setState(DisplayType::CURRENT_PLAYER_SCORE, -1, false, true, -1);
  setState(DisplayType::COMPETITION_SCORE, -1, false, true, -1);
}

void ScoreDisplay::addDisplay(DisplayType type, int deviceIndex) {
  if (!isValidType(type)) return;
  _deviceMap[static_cast<int>(type)] = deviceIndex;
}

void ScoreDisplay::clear(DisplayType type) {
  if (!isValidType(type)) return;
  int typeIdx = static_cast<int>(type);
  if (_states[typeIdx].isCleared) return;

  int deviceIndex = _deviceMap[typeIdx];
  if (deviceIndex == -1) return;
  _lc.clearDisplay(deviceIndex);

  setState(type, -1, false, true, -1);
}

void ScoreDisplay::print_number(int number, DisplayType type, bool blink)
{
  if (!isValidType(type)) return;
  int typeIdx = static_cast<int>(type);
  int deviceIndex = _deviceMap[typeIdx];
  if (deviceIndex == -1) return;

  int numberToDisplay = number;
  if (numberToDisplay > 99999) {
    numberToDisplay = 99999;
  }
  if (numberToDisplay < -9999) {
    numberToDisplay = -9999;
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
      int targetIndex = i + emptySlots;
      if (targetIndex >= 0 && targetIndex < NUM_DIGITS_PER_DISPLAY) {
        _lc.setChar(deviceIndex, targetIndex, digits[len - 1 - i], false);
      }
    }
  }

  setState(type, numberToDisplay, blink, false, targetIntensity);
}

void ScoreDisplay::setState(DisplayType type, int number, bool blink, bool isCleared, int lastIntensity) {
  if (!isValidType(type)) return;
  int typeIdx = static_cast<int>(type);
  _states[typeIdx].number = number;
  _states[typeIdx].blink = blink;
  _states[typeIdx].isCleared = isCleared;
  _states[typeIdx].lastIntensity = lastIntensity;
}
