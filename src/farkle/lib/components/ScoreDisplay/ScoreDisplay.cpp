#include "ScoreDisplay.h"
#include <Arduino.h> // Include Arduino.h for Serial.println

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

ScoreDisplay::ScoreDisplay(int csPin)
  : _csPin(csPin)
{
  for (int i = 0; i < NUM_DISPLAY_TYPES; i++) {
    _deviceMap[i] = -1;
  }
}

// MAX7219 Registers
#define MAX7219_REG_NOOP         0x00
#define MAX7219_REG_DIGIT0       0x01
#define MAX7219_REG_DECODEMODE   0x09
#define MAX7219_REG_INTENSITY    0x0A
#define MAX7219_REG_SCANLIMIT    0x0B
#define MAX7219_REG_SHUTDOWN     0x0C
#define MAX7219_REG_DISPLAYTEST  0x0F

void ScoreDisplay::max7219_write(int deviceIndex, uint8_t reg, uint8_t data) {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csPin, LOW);

  // MAX7219 devices are daisy chained.
  // We send NOOP to devices we don't want to update.
  // The first data sent ends up in the LAST device in the chain.
  for (int i = NUM_DEVICES - 1; i >= 0; i--) {
    if (i == deviceIndex) {
      SPI.transfer(reg);
      SPI.transfer(data);
    } else {
      SPI.transfer(MAX7219_REG_NOOP);
      SPI.transfer(0x00);
    }
  }

  digitalWrite(_csPin, HIGH);
  SPI.endTransaction();
}

void ScoreDisplay::max7219_setIntensity(int deviceIndex, int intensity) {
  max7219_write(deviceIndex, MAX7219_REG_INTENSITY, intensity);
}

void ScoreDisplay::max7219_clear(int deviceIndex) {
  for (int i = 0; i < 8; i++) {
    max7219_write(deviceIndex, MAX7219_REG_DIGIT0 + i, 0x00);
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
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);
  SPI.begin();

  // Initialize all devices
  for (int i = 0; i < NUM_DEVICES; i++) {
    max7219_write(i, MAX7219_REG_SHUTDOWN, 0x00); // Disable
    max7219_write(i, MAX7219_REG_DISPLAYTEST, 0x00); // No test mode
    max7219_write(i, MAX7219_REG_DECODEMODE, 0x00); // Disable BCD decoding
    max7219_write(i, MAX7219_REG_SCANLIMIT, NUM_DIGITS_PER_DISPLAY - 1); // Scan exactly the number of digits we use
    max7219_clear(i);
    max7219_setIntensity(i, SCORE_DEFAULT_INTENSITY);
    max7219_write(i, MAX7219_REG_SHUTDOWN, 0x01); // Enable
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
  max7219_clear(deviceIndex);

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
    max7219_setIntensity(deviceIndex, targetIntensity);
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

    max7219_clear(deviceIndex); // clear remaining spaces

    const int emptySlots = NUM_DIGITS_PER_DISPLAY - len;

    // MAX7219 digits are 1-indexed for the command register (MAX7219_REG_DIGIT0 + i)
    // The user noted: "The module is wired so that the left most digit is Digit 0."
    // So targetIndex 0 is the leftmost digit.
    // MAX7219 registers: 0x01 is Digit 0, 0x02 is Digit 1, etc.
    for (int i = 0; i < len; ++i) {
        int targetIndex = i + emptySlots;
        if (targetIndex >= 0 && targetIndex < NUM_DIGITS_PER_DISPLAY) {
            max7219_write(deviceIndex, MAX7219_REG_DIGIT0 + targetIndex, charToSegment(digits[len - 1 - i]));
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
