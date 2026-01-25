#include "TextDisplay.h"
#include <Wire.h> // Include Wire.h for I2C communication
#include <Arduino.h>

TextDisplay::TextDisplay() : _display(U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE))
{}

void TextDisplay::begin() {
  Wire.begin(); // Initialize I2C bus
  _display.setI2CAddress(0x3C * 2); // Explicitly set I2C address
  _display.begin();
}

void TextDisplay::print(const char* message)
{
  _display.clearBuffer();
  _display.setFont(u8g2_font_ncenB10_tr);
  int message_width = _display.getStrWidth(message);
  int x = (_display.getDisplayWidth() - message_width) / 2 + 4; // Adjusted x coordinate for SH1106 offset
  int y = (_display.getDisplayHeight() + 10) / 2; // +10 for font height approx
  _display.drawStr(x, y, message);
  _display.sendBuffer();
}

