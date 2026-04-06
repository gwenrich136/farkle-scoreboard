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
    SELECTION
};

class TextDisplayV2
{
  public:
    TextDisplayV2(int cs, int dc, int res, int blk);
    void begin();
    void print(const char* message, uint16_t hue = 0xFFFF);
    void printSelectionScreen(const char* selectionTitle, const char* selectionItem, uint16_t hue = 0xFFFF);

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

    void drawArrow(int x, int y, bool up, uint16_t color);
};

#endif
