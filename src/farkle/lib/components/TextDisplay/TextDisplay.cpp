#include "TextDisplay.h"
#include <Wire.h> // Include Wire.h for I2C communication
#include <Arduino.h>

TextDisplay::TextDisplay() : _display(U8G2_SH1106_128X64_NONAME_1_HW_I2C(U8G2_R0, U8X8_PIN_NONE))
{}

void TextDisplay::begin() {
  Serial.println("    TEXT: Calling Wire.begin()...");
  Wire.begin(); // Initialize I2C bus
  delay(100);   // Wait for I2C and Display to stabilize after power-up
  
  Serial.println("    TEXT: Setting Address...");
  _display.setI2CAddress(0x3C * 2); // Explicitly set I2C address
  
  Serial.println("    TEXT: Calling U8g2.begin()...");
  _display.begin();
  
  Serial.println("    TEXT: Init complete.");
}

void TextDisplay::print(const char* message)
{
  if (_lastMessage == message) {
    return; // Optimization: Don't redraw if text hasn't changed
  }
  _lastMessage = message;

  _display.setFont(u8g2_font_ncenB10_tr);
  int message_width = _display.getStrWidth(message);
  int x = (_display.getDisplayWidth() - message_width) / 2;
  int y = (_display.getDisplayHeight() + 10) / 2; // +10 for font height approx
  
  _display.firstPage();
  do {
    _display.drawStr(x, y, message);
  } while (_display.nextPage());
}

