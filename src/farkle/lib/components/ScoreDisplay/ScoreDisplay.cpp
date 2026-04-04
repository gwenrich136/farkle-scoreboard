#include "ScoreDisplay.h"
#include <Arduino.h> // Include Arduino.h for Serial.println

#define NUM_DEVICES 3
#define NUM_DIGITS_PER_DISPLAY 5

#define SCORE_BLINK_LOW 2
#define SCORE_BLINK_HIGH 12
#define SCORE_DEFAULT_INTENSITY 8

// Standard 7-segment encoding for MAX7219 (DP A B C D E F G)
// where A=bit6, B=bit5, etc.
static const uint8_t charToSegment(char c) {
  switch (c) {
    case '0': return 0x7E;
    case '1': return 0x30;
    case '2': return 0x6D;
    case '3': return 0x79;
    case '4': return 0x33;
    case '5': return 0x5B;
    case '6': return 0x5F;
    case '7': return 0x70;
    case '8': return 0x7F;
    case '9': return 0x7B;
    case '-': return 0x01;
    case ' ': default: return 0x00;
  }
}

// Standard FC16_HW modules are the most common generic 4-in-1 dot matrix / 7-seg displays.
// If actual hardware shows backward or inverted text, change to GENERIC_HW or PAROLA_HW.
ScoreDisplay::ScoreDisplay(int csPin)
  : _lc(MD_MAX72XX::GENERIC_HW, csPin, NUM_DEVICES)
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
  _lc.begin();
  for (int i = 0; i < NUM_DEVICES; i++) {
    _lc.control(i, MD_MAX72XX::INTENSITY, SCORE_DEFAULT_INTENSITY);
    _lc.clear(i);
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
  _lc.clear(deviceIndex);

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
    _lc.control(deviceIndex, MD_MAX72XX::INTENSITY, targetIntensity);
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
    // MD_MAX72XX typically addresses digits per module 0-7.
    // Assuming each module is a single device.
    // For setChar in MD_MAX72XX: we just clear all 8 digits, then write the ones we need.
    // NOTE: MD_MAX72XX does not have a setChar that takes (device, digit, char).
    // Instead, it maps columns 0 to (NUM_DEVICES * 8) - 1.
    // A standard 7-segment display wired as FC-16 module puts digit 0 at col 0, digit 1 at col 1, etc.
    // Let's implement printing to the correct columns for this device.
    // Each device has 8 columns (digits).

    int startCol = deviceIndex * 8;
    for (int i = 0; i < 8; i++) {
        _lc.setColumn(startCol + i, 0x00);
    }
    for (int i = 0; i < len; ++i) {
        int targetIndex = i + emptySlots;
        if (targetIndex >= 0 && targetIndex < NUM_DIGITS_PER_DISPLAY) {
            // Note: NUM_DIGITS_PER_DISPLAY is 5, but hardware digits might be mapped 0-7.
            // On a 7-segment display with GENERIC_HW, column indices directly map to digits.
            _lc.setColumn(startCol + targetIndex, charToSegment(digits[len - 1 - i]));
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
