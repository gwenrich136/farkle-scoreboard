#include "TextDisplay.h"
#include <Wire.h> // Include Wire.h for I2C communication
#include <Arduino.h>

TextDisplay::TextDisplay() : _display(U8G2_SH1106_128X64_NONAME_1_HW_I2C(U8G2_R0, U8X8_PIN_NONE)), _currentMode(DisplayMode::NONE)
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
  if (_currentMode == DisplayMode::MESSAGE && _lastMessage == message) {
    return; // Optimization: Don't redraw if text hasn't changed
  }
  _currentMode = DisplayMode::MESSAGE;
  _lastMessage = message;

  _display.setFont(TEXT_DISPLAY_MAIN_FONT);
  int message_width = _display.getStrWidth(message);
  int x = (_display.getDisplayWidth() - message_width) / 2;
  int y = (_display.getDisplayHeight() + 10) / 2; // +10 for font height approx
  
  _display.firstPage();
  do {
    _display.drawStr(x, y, message);
  } while (_display.nextPage());
}

void TextDisplay::printSelectionScreen(const char* selectionTitle, const char* selectionItem)
{
    if (_currentMode == DisplayMode::SELECTION && _lastTitle == selectionTitle && _lastItem == selectionItem) {
        return;
    }
    _currentMode = DisplayMode::SELECTION;
    _lastTitle = selectionTitle;
    _lastItem = selectionItem;

    int centerX = _display.getDisplayWidth() / 2;
    // Calculate vertical positions
    int titleY = 10; // Top margin
    int itemY = (_display.getDisplayHeight() + 10) / 2 + 8; // Lower center

    // Draw Arrows
    int arrowSize = TEXT_DISPLAY_ARROW_SIZE;
    int arrowSpacing = TEXT_DISPLAY_ARROW_SPACING;

    // Up arrow above item (approx 15px text height)
    // Center of arrow is at itemY - 15 - spacing - size/2
    int upArrowCenterY = itemY - 15 - arrowSpacing - arrowSize/2;
    // Down arrow below item (approx baseline)
    int downArrowCenterY = itemY + arrowSpacing + arrowSize/2;

    _display.firstPage();
    do {
        // Draw Title
        _display.setFont(TEXT_DISPLAY_TITLE_FONT);
        int titleWidth = _display.getStrWidth(selectionTitle);
        int titleX = (_display.getDisplayWidth() - titleWidth) / 2;
        _display.drawStr(titleX, titleY, selectionTitle);

        // Draw Item
        _display.setFont(TEXT_DISPLAY_MAIN_FONT);
        int itemWidth = _display.getStrWidth(selectionItem);
        int itemX = (_display.getDisplayWidth() - itemWidth) / 2;
        _display.drawStr(itemX, itemY, selectionItem);

        // Draw Arrows
        drawArrow(centerX, upArrowCenterY, true);
        drawArrow(centerX, downArrowCenterY, false);

    } while (_display.nextPage());
}

void TextDisplay::drawArrow(int x, int y, bool up) {
    int size = TEXT_DISPLAY_ARROW_SIZE / 2;
    if (up) {
        // Tip (x, y-size), Left (x-size, y+size), Right (x+size, y+size)
        // User requested diagonal lines meeting at right angle "like ^"
        // This means a caret.
        _display.drawLine(x - size, y + size, x, y - size);
        _display.drawLine(x + size, y + size, x, y - size);
    } else {
        // Tip (x, y+size), Left (x-size, y-size), Right (x+size, y-size)
        // "v"
        _display.drawLine(x - size, y - size, x, y + size);
        _display.drawLine(x + size, y - size, x, y + size);
    }
}
