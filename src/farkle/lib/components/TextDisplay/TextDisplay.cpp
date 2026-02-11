#include "TextDisplay.h"
#include <Wire.h> // Include Wire.h for I2C communication
#include <Arduino.h>
#include <string.h>

TextDisplay::TextDisplay() :
    _display(U8G2_SH1106_128X64_NONAME_1_HW_I2C(U8G2_R0, U8X8_PIN_NONE)),
    _currentMode(DisplayMode::NONE),
    _lastActiveIndex(-1)
{
    _lastMessage[0] = '\0';
    _lastTitle[0] = '\0';
    _lastItem[0] = '\0';
    _lastSubtitle[0] = '\0';
    _lastLeftSubtitle[0] = '\0';
    _lastRightSubtitle[0] = '\0';
}

void TextDisplay::begin() {
  Serial.println("    TEXT: Calling Wire.begin()...");
  Wire.begin(); // Initialize I2C bus
  delay(100);   // Wait for I2C and Display to stabilize after power-up
  
  Serial.println("    TEXT: Setting Address...");
  _display.setI2CAddress(0x3C * 2); // Explicitly set I2C address
  
  Serial.println("    TEXT: Calling U8g2.begin()...");
  if (_display.begin()) {
      Serial.println("    TEXT: U8g2.begin() succeeded.");
  } else {
      Serial.println("    TEXT: U8g2.begin() FAILED!");
  }
  
  // Configure deterministic font behavior
  _display.setFontPosTop();
  _display.setFontRefHeightExtendedText();

  Serial.println("    TEXT: Init complete.");
}

bool TextDisplay::shouldRedraw(DisplayMode mode, const char* s1, const char* s2, const char* s3, int index) {
    bool redraw = false;
    if (_currentMode != mode) redraw = true;

    // Mapping of s1, s2, s3 depends on mode, but we can just check all provided ones
    // and compare against their corresponding "last" fields based on the mode.
    // To keep it simple, let's just map them consistently for each mode:
    // MESSAGE: s1=message
    // SELECTION: s1=title, s2=item
    // TITLE: s1=title
    // TITLE_SUBTITLE: s1=title, s2=subtitle
    // TITLE_SUBTITLES: s1=title, s2=left, s3=right
    // CHARACTER_INPUT: s1=name, index=activeIndex

    if (mode == DisplayMode::MESSAGE) {
        if (s1 && strcmp(_lastMessage, s1) != 0) redraw = true;
    } else if (mode == DisplayMode::SELECTION) {
        if (s1 && strcmp(_lastTitle, s1) != 0) redraw = true;
        if (s2 && strcmp(_lastItem, s2) != 0) redraw = true;
    } else if (mode == DisplayMode::TITLE) {
        if (s1 && strcmp(_lastTitle, s1) != 0) redraw = true;
    } else if (mode == DisplayMode::TITLE_SUBTITLE) {
        if (s1 && strcmp(_lastTitle, s1) != 0) redraw = true;
        if (s2 && strcmp(_lastSubtitle, s2) != 0) redraw = true;
    } else if (mode == DisplayMode::TITLE_SUBTITLES) {
        if (s1 && strcmp(_lastTitle, s1) != 0) redraw = true;
        if (s2 && strcmp(_lastLeftSubtitle, s2) != 0) redraw = true;
        if (s3 && strcmp(_lastRightSubtitle, s3) != 0) redraw = true;
    } else if (mode == DisplayMode::CHARACTER_INPUT) {
        if (s1 && strcmp(_lastMessage, s1) != 0) redraw = true;
        if (index != _lastActiveIndex) redraw = true;
    }

    if (redraw) {
        Serial.print("    TEXT: Redraw triggered for mode ");
        Serial.println((int)mode);
        _currentMode = mode;
        if (mode == DisplayMode::MESSAGE || mode == DisplayMode::CHARACTER_INPUT) {
            if (s1) { strncpy(_lastMessage, s1, TEXT_DISPLAY_BUFFER_SIZE - 1); _lastMessage[TEXT_DISPLAY_BUFFER_SIZE - 1] = '\0'; }
        }
        if (mode == DisplayMode::SELECTION || mode == DisplayMode::TITLE || mode == DisplayMode::TITLE_SUBTITLE || mode == DisplayMode::TITLE_SUBTITLES) {
            if (s1) { strncpy(_lastTitle, s1, TEXT_DISPLAY_BUFFER_SIZE - 1); _lastTitle[TEXT_DISPLAY_BUFFER_SIZE - 1] = '\0'; }
        }
        if (mode == DisplayMode::SELECTION) {
            if (s2) { strncpy(_lastItem, s2, TEXT_DISPLAY_BUFFER_SIZE - 1); _lastItem[TEXT_DISPLAY_BUFFER_SIZE - 1] = '\0'; }
        }
        if (mode == DisplayMode::TITLE_SUBTITLE) {
            if (s2) { strncpy(_lastSubtitle, s2, TEXT_DISPLAY_BUFFER_SIZE - 1); _lastSubtitle[TEXT_DISPLAY_BUFFER_SIZE - 1] = '\0'; }
        }
        if (mode == DisplayMode::TITLE_SUBTITLES) {
            if (s2) { strncpy(_lastLeftSubtitle, s2, TEXT_DISPLAY_BUFFER_SIZE - 1); _lastLeftSubtitle[TEXT_DISPLAY_BUFFER_SIZE - 1] = '\0'; }
            if (s3) { strncpy(_lastRightSubtitle, s3, TEXT_DISPLAY_BUFFER_SIZE - 1); _lastRightSubtitle[TEXT_DISPLAY_BUFFER_SIZE - 1] = '\0'; }
        }
        _lastActiveIndex = index;
    }
    return redraw;
}

void TextDisplay::print(const char* message)
{
  if (!shouldRedraw(DisplayMode::MESSAGE, message)) {
    return;
  }

  _display.setFont(TEXT_DISPLAY_MAIN_FONT);
  int message_width = _display.getStrWidth(message);
  int x = (_display.getDisplayWidth() - message_width) / 2;
  if (x < 0) x = 0;
  int y = (_display.getDisplayHeight() - TEXT_DISPLAY_MAIN_HEIGHT) / 2;
  
  _display.firstPage();
  do {
    _display.drawStr(x, y, message);
  } while (_display.nextPage());
}

void TextDisplay::printSelectionScreen(const char* selectionTitle, const char* selectionItem)
{
    if (!shouldRedraw(DisplayMode::SELECTION, selectionTitle, selectionItem)) {
        return;
    }

    _display.setFont(TEXT_DISPLAY_TITLE_FONT);
    int titleWidth = _display.getStrWidth(selectionTitle);
    int titleX = (_display.getDisplayWidth() - titleWidth) / 2;
    if (titleX < 0) titleX = 0;

    _display.setFont(TEXT_DISPLAY_MAIN_FONT);
    int itemWidth = _display.getStrWidth(selectionItem);
    int itemX = (_display.getDisplayWidth() - itemWidth) / 2;
    if (itemX < 0) itemX = 0;
    int centerX = _display.getDisplayWidth() / 2;

    int titleY = 2;
    int itemY = 38;

    int arrowHeight = TEXT_DISPLAY_ARROW_HEIGHT;
    int arrowSpacing = TEXT_DISPLAY_ARROW_SPACING;
    int upArrowCenterY = itemY - arrowSpacing - arrowHeight/2;
    int downArrowCenterY = itemY + TEXT_DISPLAY_MAIN_HEIGHT + arrowSpacing + arrowHeight/2;

    _display.firstPage();
    do {
        _display.setFont(TEXT_DISPLAY_TITLE_FONT);
        _display.drawStr(titleX, titleY, selectionTitle);
        _display.setFont(TEXT_DISPLAY_MAIN_FONT);
        _display.drawStr(itemX, itemY, selectionItem);
        drawArrow(centerX, upArrowCenterY, true);
        drawArrow(centerX, downArrowCenterY, false);
    } while (_display.nextPage());
}

void TextDisplay::displayTitle(const char* title) {
    if (!shouldRedraw(DisplayMode::TITLE, title)) return;

    _display.setFont(TEXT_DISPLAY_LARGE_FONT);
    int w = _display.getStrWidth(title);
    int x = (_display.getDisplayWidth() - w) / 2;
    if (x < 0) x = 0;
    int y = (_display.getDisplayHeight() - TEXT_DISPLAY_LARGE_HEIGHT) / 2;

    _display.firstPage();
    do {
        _display.drawStr(x, y, title);
    } while (_display.nextPage());
}

void TextDisplay::displayTitleWithSubtitle(const char* title, const char* subtitle) {
    if (!shouldRedraw(DisplayMode::TITLE_SUBTITLE, title, subtitle)) return;

    _display.setFont(TEXT_DISPLAY_LARGE_FONT);
    int tw = _display.getStrWidth(title);
    int tx = (_display.getDisplayWidth() - tw) / 2;
    if (tx < 0) tx = 0;

    _display.setFont(TEXT_DISPLAY_TITLE_FONT);
    int sw = _display.getStrWidth(subtitle);
    int sx = (_display.getDisplayWidth() - sw) / 2;
    if (sx < 0) sx = 0;

    int ty = 10;
    int sy = 45;

    _display.firstPage();
    do {
        _display.setFont(TEXT_DISPLAY_LARGE_FONT);
        _display.drawStr(tx, ty, title);
        _display.setFont(TEXT_DISPLAY_TITLE_FONT);
        _display.drawStr(sx, sy, subtitle);
    } while (_display.nextPage());
}

void TextDisplay::displayTitleWithSubtitles(const char* title, const char* leftSubtitle, const char* rightSubtitle) {
    if (!shouldRedraw(DisplayMode::TITLE_SUBTITLES, title, leftSubtitle, rightSubtitle)) return;

    _display.setFont(TEXT_DISPLAY_LARGE_FONT);
    int tw = _display.getStrWidth(title);
    int tx = (_display.getDisplayWidth() - tw) / 2;
    if (tx < 0) tx = 0;

    _display.setFont(TEXT_DISPLAY_TITLE_FONT);
    // leftSubtitle at x=2, rightSubtitle at x = width - sw - 2
    int swRight = _display.getStrWidth(rightSubtitle);
    int sxLeft = 2;
    int sxRight = _display.getDisplayWidth() - swRight - 2;

    int ty = 10;
    int sy = 45;

    _display.firstPage();
    do {
        _display.setFont(TEXT_DISPLAY_LARGE_FONT);
        _display.drawStr(tx, ty, title);
        _display.setFont(TEXT_DISPLAY_TITLE_FONT);
        _display.drawStr(sxLeft, sy, leftSubtitle);
        _display.drawStr(sxRight, sy, rightSubtitle);
    } while (_display.nextPage());
}

void TextDisplay::displayCharacterInput(const char* currentName, int activeIndex) {
    if (!shouldRedraw(DisplayMode::CHARACTER_INPUT, currentName, nullptr, nullptr, activeIndex)) return;

    int len = strlen(currentName);
    if (activeIndex < 0 || activeIndex >= len) return;

    char activeChar[2] = { currentName[activeIndex], '\0' };

    _display.setFont(TEXT_DISPLAY_LARGE_FONT);
    int aw = _display.getStrWidth(activeChar);
    int ax = (_display.getDisplayWidth() - aw) / 2;
    int ay = (_display.getDisplayHeight() - TEXT_DISPLAY_LARGE_HEIGHT) / 2;

    // Arrows for active char
    int arrowHeight = TEXT_DISPLAY_ARROW_HEIGHT;
    int arrowSpacing = TEXT_DISPLAY_ARROW_SPACING;
    int centerX = _display.getDisplayWidth() / 2;
    int upArrowY = ay - arrowSpacing - arrowHeight/2;
    int downArrowY = ay + TEXT_DISPLAY_LARGE_HEIGHT + arrowSpacing + arrowHeight/2;

    _display.firstPage();
    do {
        // Draw active character
        _display.setFont(TEXT_DISPLAY_LARGE_FONT);
        _display.drawStr(ax, ay, activeChar);

        // Draw Arrows
        drawArrow(centerX, upArrowY, true);
        drawArrow(centerX, downArrowY, false);

        // Draw left part of the name
        _display.setFont(TEXT_DISPLAY_TITLE_FONT);
        if (activeIndex > 0) {
            char leftPart[64];
            strncpy(leftPart, currentName, activeIndex);
            leftPart[activeIndex] = '\0';
            int lw = _display.getStrWidth(leftPart);
            _display.drawStr(ax - lw - 4, ay + (TEXT_DISPLAY_LARGE_HEIGHT - TEXT_DISPLAY_TITLE_HEIGHT)/2, leftPart);
        }

        // Draw right part of the name
        if (activeIndex < len - 1) {
            const char* rightPart = &currentName[activeIndex + 1];
            _display.drawStr(ax + aw + 4, ay + (TEXT_DISPLAY_LARGE_HEIGHT - TEXT_DISPLAY_TITLE_HEIGHT)/2, rightPart);
        }
    } while (_display.nextPage());
}

void TextDisplay::drawArrow(int x, int y, bool up) {
    int halfW = TEXT_DISPLAY_ARROW_WIDTH / 2;
    int halfH = TEXT_DISPLAY_ARROW_HEIGHT / 2;
    
    if (up) {
        _display.drawLine(x - halfW, y + halfH, x, y - halfH);
        _display.drawLine(x + halfW, y + halfH, x, y - halfH);
    } else {
        _display.drawLine(x - halfW, y - halfH, x, y + halfH);
        _display.drawLine(x + halfW, y - halfH, x, y + halfH);
    }
}
