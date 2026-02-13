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
  
  // Configure deterministic font behavior
  _display.setFontPosTop();
  _display.setFontRefHeightExtendedText();

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
  int y = (_display.getDisplayHeight() - TEXT_DISPLAY_MAIN_HEIGHT) / 2;
  
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

    // 1. Pre-calculate horizontal alignment
    _display.setFont(TEXT_DISPLAY_TITLE_FONT);
    int titleWidth = _display.getStrWidth(selectionTitle);
    int titleX = (_display.getDisplayWidth() - titleWidth) / 2;

    _display.setFont(TEXT_DISPLAY_MAIN_FONT);
    int itemWidth = _display.getStrWidth(selectionItem);
    int itemX = (_display.getDisplayWidth() - itemWidth) / 2;
    int centerX = _display.getDisplayWidth() / 2;

    // 2. Pre-calculate vertical positioning (Deterministic)
    int titleY = 2; // Fixed top margin (with setFontPosTop)
    int itemY = 38; // Pushed down to create more space from title

    // 3. Pre-calculate arrow geometry
    int arrowWidth = TEXT_DISPLAY_ARROW_WIDTH;
    int arrowHeight = TEXT_DISPLAY_ARROW_HEIGHT;
    int arrowSpacing = TEXT_DISPLAY_ARROW_SPACING;
    
    // Up arrow is above the item text
    int upArrowCenterY = itemY - arrowSpacing - arrowHeight/2;
    // Down arrow is below the item text
    int downArrowCenterY = itemY + TEXT_DISPLAY_MAIN_HEIGHT + arrowSpacing + arrowHeight/2;

    _display.firstPage();
    do {
        // Draw Title
        _display.setFont(TEXT_DISPLAY_TITLE_FONT);
        _display.drawStr(titleX, titleY, selectionTitle);

        // Draw Item
        _display.setFont(TEXT_DISPLAY_MAIN_FONT);
        _display.drawStr(itemX, itemY, selectionItem);

        // Draw Arrows
        drawArrow(centerX, upArrowCenterY, true);
        drawArrow(centerX, downArrowCenterY, false);

    } while (_display.nextPage());
}

void TextDisplay::drawArrow(int x, int y, bool up) {
    int halfW = TEXT_DISPLAY_ARROW_WIDTH / 2;
    int halfH = TEXT_DISPLAY_ARROW_HEIGHT / 2;
    
    if (up) {
        // Tip (x, y-halfH), Left (x-halfW, y+halfH), Right (x+halfW, y+halfH)
        _display.drawLine(x - halfW, y + halfH, x, y - halfH);
        _display.drawLine(x + halfW, y + halfH, x, y - halfH);
    } else {
        // Tip (x, y+halfH), Left (x-halfW, y-halfH), Right (x+halfW, y-halfH)
        _display.drawLine(x - halfW, y - halfH, x, y + halfH);
        _display.drawLine(x + halfW, y - halfH, x, y + halfH);
    }
}
