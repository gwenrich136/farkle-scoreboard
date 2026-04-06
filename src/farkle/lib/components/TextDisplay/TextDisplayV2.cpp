#include "TextDisplayV2.h"
#include <Fonts/FreeSans18pt7b.h>
#include <math.h>
#include <cstring>

// Arrow geometry
#define ARROW_WIDTH 20
#define ARROW_HEIGHT 12
#define ARROW_SPACING 15

TextDisplayV2::TextDisplayV2(int cs, int dc, int res, int blk)
    : _display(cs, dc, res), _blkPin(blk), _currentMode(DisplayModeV2::NONE), _lastHue(0xFFFF)
{
    _lastMessage[0] = '\0';
    _lastTitle[0] = '\0';
    _lastItem[0] = '\0';
}

void TextDisplayV2::begin() {
  Serial.println("    TEXTV2: Initializing ST7789 LCD...");

  if (_blkPin >= 0) {
      pinMode(_blkPin, OUTPUT);
      digitalWrite(_blkPin, HIGH); // Backlight 100%
  }

  _display.init(240, 240); // Standard init for 240x240 IPS
  _display.setRotation(2); // Adjust rotation as needed for hardware orientation
  _display.fillScreen(ST77XX_BLACK);

  Serial.println("    TEXTV2: Init complete.");
}

void TextDisplayV2::print(const char* message, uint16_t hue)
{
  if (_currentMode == DisplayModeV2::MESSAGE && strcmp(_lastMessage, message) == 0 && _lastHue == hue) {
    return; // State cache
  }
  _currentMode = DisplayModeV2::MESSAGE;
  strncpy(_lastMessage, message, sizeof(_lastMessage) - 1);
  _lastMessage[sizeof(_lastMessage) - 1] = '\0';
  _lastHue = hue;

  _display.fillScreen(ST77XX_BLACK);
  _display.setFont(&FreeSans18pt7b);

  uint16_t textColor = (hue == 0xFFFF) ? ST77XX_WHITE : colorHSVtoRGB565(hue);
  _display.setTextColor(textColor);

  int16_t x1, y1;
  uint16_t w, h;
  _display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);

  int16_t x = LCD_CENTER_X - (w / 2);
  int16_t y = LCD_CENTER_Y + (h / 2); // FreeType fonts draw from baseline, so we adjust Y down

  _display.setCursor(x, y);
  _display.print(message);
}

void TextDisplayV2::printSelectionScreen(const char* selectionTitle, const char* selectionItem, uint16_t hue)
{
    if (_currentMode == DisplayModeV2::SELECTION &&
        strcmp(_lastTitle, selectionTitle) == 0 &&
        strcmp(_lastItem, selectionItem) == 0 &&
        _lastHue == hue) {
        return; // State cache
    }
    _currentMode = DisplayModeV2::SELECTION;
    strncpy(_lastTitle, selectionTitle, sizeof(_lastTitle) - 1);
    _lastTitle[sizeof(_lastTitle) - 1] = '\0';
    strncpy(_lastItem, selectionItem, sizeof(_lastItem) - 1);
    _lastItem[sizeof(_lastItem) - 1] = '\0';
    _lastHue = hue;

    _display.fillScreen(ST77XX_BLACK);
    _display.setFont(&FreeSans18pt7b);

    // Resolve color
    uint16_t itemColor = (hue == 0xFFFF) ? ST77XX_WHITE : colorHSVtoRGB565(hue);

    int16_t x1, y1;
    uint16_t w, h;

    // Title
    _display.getTextBounds(selectionTitle, 0, 0, &x1, &y1, &w, &h);
    int16_t titleX = LCD_CENTER_X - (w / 2);
    int16_t titleY = 40; // Fixed top margin
    _display.setTextColor(ST77XX_WHITE);
    _display.setCursor(titleX, titleY);
    _display.print(selectionTitle);

    // Item
    _display.getTextBounds(selectionItem, 0, 0, &x1, &y1, &w, &h);
    int16_t itemX = LCD_CENTER_X - (w / 2);
    int16_t itemY = LCD_CENTER_Y + (h / 2); // Center vertically
    _display.setTextColor(itemColor);
    _display.setCursor(itemX, itemY);
    _display.print(selectionItem);

    // Arrows
    int upArrowY = itemY - h - ARROW_SPACING;
    int downArrowY = itemY + ARROW_SPACING;

    drawArrow(LCD_CENTER_X, upArrowY, true, ST77XX_WHITE);
    drawArrow(LCD_CENTER_X, downArrowY, false, ST77XX_WHITE);
}

void TextDisplayV2::drawArrow(int x, int y, bool up, uint16_t color) {
    int halfW = ARROW_WIDTH / 2;
    int halfH = ARROW_HEIGHT / 2;
    int thickness = 3;

    if (up) {
        for (int i = 0; i < thickness; i++) {
            // Tip (x, y-halfH+i), Left (x-halfW, y+halfH+i), Right (x+halfW, y+halfH+i)
            _display.drawLine(x - halfW, y + halfH + i, x, y - halfH + i, color);
            _display.drawLine(x + halfW, y + halfH + i, x, y - halfH + i, color);
        }
    } else {
        for (int i = 0; i < thickness; i++) {
            // Tip (x, y+halfH-i), Left (x-halfW, y-halfH-i), Right (x+halfW, y-halfH-i)
            _display.drawLine(x - halfW, y - halfH - i, x, y + halfH - i, color);
            _display.drawLine(x + halfW, y - halfH - i, x, y + halfH - i, color);
        }
    }
}

uint16_t TextDisplayV2::colorHSVtoRGB565(uint16_t hue) {
    // Basic conversion from hue (0-65535) to RGB565 with max saturation & value
    // We do not rely on standard HSV-to-RGB library here to avoid external heavy dependencies.
    // The following code efficiently maps 0-65535 to an RGB565 short using standard algorithms.
    float h = (hue / 65535.0f) * 360.0f;
    float c = 1.0f;
    float x = c * (1.0f - abs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = 0.0f;

    float r = 0, g = 0, b = 0;
    if (h >= 0 && h < 60) { r = c; g = x; b = 0; }
    else if (h >= 60 && h < 120) { r = x; g = c; b = 0; }
    else if (h >= 120 && h < 180) { r = 0; g = c; b = x; }
    else if (h >= 180 && h < 240) { r = 0; g = x; b = c; }
    else if (h >= 240 && h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    uint8_t R5 = (r + m) * 31;
    uint8_t G6 = (g + m) * 63;
    uint8_t B5 = (b + m) * 31;

    return (R5 << 11) | (G6 << 5) | B5;
}
