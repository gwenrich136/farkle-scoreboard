#ifndef TextDisplayV2_h
#define TextDisplayV2_h

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Display resolution
#define LCD_WIDTH 240
#define LCD_HEIGHT 240
#define LCD_CENTER_X (LCD_WIDTH / 2)
#define LCD_CENTER_Y (LCD_HEIGHT / 2)

// Colors
#define ST77XX_BLACK 0x0000
#define ST77XX_WHITE 0xFFFF

enum class DisplayModeV2 {
    NONE,
    MESSAGE,
    SELECTION,
    HEAD_TO_HEAD
};

class TextDisplayV2
{
  public:
    TextDisplayV2(int cs, int dc, int res, int blk);
    void begin();
    void print(const char* message, uint16_t hue = 0xFFFF);
    void printSelectionScreen(const char* selectionTitle, const char* selectionItem, uint16_t hue = 0xFFFF);
    void printHeadToHeadScreen(const char* p1Place, const char* p1Name, uint16_t p1Hue, const char* p2Place, const char* p2Name, uint16_t p2Hue);

    // Color conversion utility
    static uint16_t colorHSVtoRGB565(uint16_t hue);

  private:
    Adafruit_ST7789 _display;
    int _blkPin;

    // State tracking
    DisplayModeV2 _currentMode;
    char _lastMessage[32];
    char _lastTitle[32];
    char _lastItem[32];
    uint16_t _lastHue;

    // Head-to-Head state tracking
    char _lastP1Place[16];
    char _lastP1Name[32];
    uint16_t _lastP1Hue;
    char _lastP2Place[16];
    char _lastP2Name[32];
    uint16_t _lastP2Hue;

    void eraseOldTextBoundingBox(const char* text);
    void eraseAlignedPlaceAndNameBoundingBox(const char* place, const char* name, const GFXfont* nameFont, const GFXfont* placeFont, int16_t baseCenterY);
    void drawAlignedPlaceAndName(const char* place, const char* name, uint16_t color, const GFXfont* nameFont, const GFXfont* placeFont, int16_t baseCenterY, int16_t& outY, uint16_t& outH);
    void drawSelectionTitle(const char* title);
    void drawSelectionItem(const char* item, uint16_t color, int16_t& itemY, uint16_t& itemH);
    void drawSelectionArrows(int16_t itemY, uint16_t itemH);
    void drawArrow(int x, int y, bool up, uint16_t color);
};

#endif
