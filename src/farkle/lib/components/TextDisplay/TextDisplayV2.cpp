#include "TextDisplayV2.h"
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
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
    _lastP1Place[0] = '\0';
    _lastP1Name[0] = '\0';
    _lastP1Hue = 0xFFFF;
    _lastP2Place[0] = '\0';
    _lastP2Name[0] = '\0';
    _lastP2Hue = 0xFFFF;
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
        return;
    }

    _display.setFont(&FreeSans18pt7b);

    if (_currentMode == DisplayModeV2::MESSAGE) {
        eraseOldTextBoundingBox(_lastMessage);
    } else {
        _display.fillScreen(ST77XX_BLACK);
    }

    _currentMode = DisplayModeV2::MESSAGE;
    strncpy(_lastMessage, message, sizeof(_lastMessage) - 1);
    _lastMessage[sizeof(_lastMessage) - 1] = '\0';
    _lastHue = hue;

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
        return;
    }

    _display.setFont(&FreeSans18pt7b);

    bool titleChanged = strcmp(_lastTitle, selectionTitle) != 0;
    bool modeChanged = _currentMode != DisplayModeV2::SELECTION;
    bool needsFullRedraw = modeChanged || titleChanged;

    if (!needsFullRedraw) {
        eraseOldTextBoundingBox(_lastItem);
    } else {
        _display.fillScreen(ST77XX_BLACK);
    }

    _currentMode = DisplayModeV2::SELECTION;
    strncpy(_lastTitle, selectionTitle, sizeof(_lastTitle) - 1);
    _lastTitle[sizeof(_lastTitle) - 1] = '\0';
    strncpy(_lastItem, selectionItem, sizeof(_lastItem) - 1);
    _lastItem[sizeof(_lastItem) - 1] = '\0';
    _lastHue = hue;

    uint16_t itemColor = (hue == 0xFFFF) ? ST77XX_WHITE : colorHSVtoRGB565(hue);
    int16_t itemY;
    uint16_t itemH;

    if (needsFullRedraw) {
        drawSelectionTitle(selectionTitle);
    }

    drawSelectionItem(selectionItem, itemColor, itemY, itemH);

    if (needsFullRedraw) {
        drawSelectionArrows(itemY, itemH);
    }
}

void TextDisplayV2::printHeadToHeadScreen(const char* p1Place, const char* p1Name, uint16_t p1Hue, const char* p2Place, const char* p2Name, uint16_t p2Hue)
{
    if (_currentMode == DisplayModeV2::HEAD_TO_HEAD &&
        strcmp(_lastP1Place, p1Place) == 0 &&
        strcmp(_lastP1Name, p1Name) == 0 &&
        _lastP1Hue == p1Hue &&
        strcmp(_lastP2Place, p2Place) == 0 &&
        strcmp(_lastP2Name, p2Name) == 0 &&
        _lastP2Hue == p2Hue) {
        return;
    }

    bool p1Changed = (strcmp(_lastP1Place, p1Place) != 0) || (strcmp(_lastP1Name, p1Name) != 0) || (_lastP1Hue != p1Hue);
    bool p2Changed = (strcmp(_lastP2Place, p2Place) != 0) || (strcmp(_lastP2Name, p2Name) != 0) || (_lastP2Hue != p2Hue);
    bool modeChanged = _currentMode != DisplayModeV2::HEAD_TO_HEAD;

    if (modeChanged) {
        _display.fillScreen(ST77XX_BLACK);
    } else {
        if (p1Changed) {
            eraseAlignedPlaceAndNameBoundingBox(_lastP1Place, _lastP1Name, &FreeSans18pt7b, &FreeSans12pt7b, LCD_HEIGHT / 4);
        }
        if (p2Changed) {
            eraseAlignedPlaceAndNameBoundingBox(_lastP2Place, _lastP2Name, &FreeSans12pt7b, &FreeSans9pt7b, (LCD_HEIGHT / 4) * 3);
            // Re-draw arrows when p2 changes because its bounding box erase might clip them
            // Or we just rely on drawSelectionArrows redrawing cleanly. Let's let the helper redraw it below.
        }
    }

    _currentMode = DisplayModeV2::HEAD_TO_HEAD;
    strncpy(_lastP1Place, p1Place, sizeof(_lastP1Place) - 1); _lastP1Place[sizeof(_lastP1Place) - 1] = '\0';
    strncpy(_lastP1Name, p1Name, sizeof(_lastP1Name) - 1); _lastP1Name[sizeof(_lastP1Name) - 1] = '\0';
    _lastP1Hue = p1Hue;
    strncpy(_lastP2Place, p2Place, sizeof(_lastP2Place) - 1); _lastP2Place[sizeof(_lastP2Place) - 1] = '\0';
    strncpy(_lastP2Name, p2Name, sizeof(_lastP2Name) - 1); _lastP2Name[sizeof(_lastP2Name) - 1] = '\0';
    _lastP2Hue = p2Hue;

    uint16_t c1 = (p1Hue == 0xFFFF) ? ST77XX_WHITE : colorHSVtoRGB565(p1Hue);
    uint16_t c2 = (p2Hue == 0xFFFF) ? ST77XX_WHITE : colorHSVtoRGB565(p2Hue);

    int16_t dummyY;
    uint16_t dummyH;

    if (modeChanged || p1Changed) {
        drawAlignedPlaceAndName(p1Place, p1Name, c1, &FreeSans18pt7b, &FreeSans12pt7b, LCD_HEIGHT / 4, dummyY, dummyH);
    }

    if (modeChanged || p2Changed) {
        int16_t p2Y;
        uint16_t p2H;
        drawAlignedPlaceAndName(p2Place, p2Name, c2, &FreeSans12pt7b, &FreeSans9pt7b, (LCD_HEIGHT / 4) * 3, p2Y, p2H);
        drawSelectionArrows(p2Y, p2H);
    }
}

void TextDisplayV2::eraseOldTextBoundingBox(const char* text) {
    int16_t oldX1, oldY1;
    uint16_t oldW, oldH;
    _display.getTextBounds(text, 0, 0, &oldX1, &oldY1, &oldW, &oldH);
    int16_t oldX = LCD_CENTER_X - (oldW / 2);
    int16_t oldY = LCD_CENTER_Y + (oldH / 2);
    _display.fillRect(oldX + oldX1, oldY + oldY1, oldW, oldH, ST77XX_BLACK);
}

void TextDisplayV2::eraseAlignedPlaceAndNameBoundingBox(const char* place, const char* name, const GFXfont* nameFont, const GFXfont* placeFont, int16_t baseCenterY) {
    int16_t nameX1, nameY1, placeX1, placeY1;
    uint16_t nameW, nameH, placeW, placeH;

    _display.setFont(nameFont);
    _display.getTextBounds(name, 0, 0, &nameX1, &nameY1, &nameW, &nameH);

    _display.setFont(placeFont);
    _display.getTextBounds(place, 0, 0, &placeX1, &placeY1, &placeW, &placeH);

    int16_t nameX = LCD_CENTER_X - (nameW / 2);
    int16_t nameY = baseCenterY + (nameH / 2); // Baseline for name

    // Erase Name
    _display.fillRect(nameX + nameX1, nameY + nameY1, nameW, nameH, ST77XX_BLACK);

    int16_t placeX = nameX; // Left aligned with name
    int16_t placeY = nameY - nameH - 5; // Above the name bounding box

    // Erase Place
    _display.fillRect(placeX + placeX1, placeY + placeY1, placeW, placeH, ST77XX_BLACK);
}

void TextDisplayV2::drawAlignedPlaceAndName(const char* place, const char* name, uint16_t color, const GFXfont* nameFont, const GFXfont* placeFont, int16_t baseCenterY, int16_t& outY, uint16_t& outH) {
    int16_t nameX1, nameY1, placeX1, placeY1;
    uint16_t nameW, nameH, placeW, placeH;

    _display.setFont(nameFont);
    _display.getTextBounds(name, 0, 0, &nameX1, &nameY1, &nameW, &nameH);

    _display.setFont(placeFont);
    _display.getTextBounds(place, 0, 0, &placeX1, &placeY1, &placeW, &placeH);

    int16_t nameX = LCD_CENTER_X - (nameW / 2);
    int16_t nameY = baseCenterY + (nameH / 2);

    int16_t placeX = nameX; // Left aligned with name
    int16_t placeY = nameY - nameH - 5; // 5px padding above name

    _display.setTextColor(color);

    // Draw Place
    _display.setFont(placeFont);
    _display.setCursor(placeX, placeY);
    _display.print(place);

    // Draw Name
    _display.setFont(nameFont);
    _display.setCursor(nameX, nameY);
    _display.print(name);

    // For bounding arrows around this block, we treat the block's center/bounds based on the name
    // since the arrows surround the central element
    outY = nameY;
    outH = nameH + placeH + 5; // total block height approximation
}

void TextDisplayV2::drawSelectionTitle(const char* title) {
    int16_t x1, y1;
    uint16_t w, h;
    _display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    int16_t titleX = LCD_CENTER_X - (w / 2);
    int16_t titleY = 40; // Fixed top margin
    _display.setTextColor(ST77XX_WHITE);
    _display.setCursor(titleX, titleY);
    _display.print(title);
}

void TextDisplayV2::drawSelectionItem(const char* item, uint16_t color, int16_t& itemY, uint16_t& itemH) {
    int16_t x1, y1;
    uint16_t w;
    _display.getTextBounds(item, 0, 0, &x1, &y1, &w, &itemH);
    int16_t itemX = LCD_CENTER_X - (w / 2);
    itemY = LCD_CENTER_Y + (itemH / 2); // Center vertically
    _display.setTextColor(color);
    _display.setCursor(itemX, itemY);
    _display.print(item);
}

void TextDisplayV2::drawSelectionArrows(int16_t itemY, uint16_t itemH) {
    // Offset slightly higher for the top arrow to cover the place text as well
    int upArrowY = itemY - itemH - ARROW_SPACING + 5;
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
