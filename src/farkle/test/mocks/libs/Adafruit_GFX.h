#ifndef _ADAFRUIT_GFX_H
#define _ADAFRUIT_GFX_H

#include <stdint.h>
#include <string>
#include <vector>

// Mock Font structures
typedef struct {
    uint16_t bitmapOffset;     ///< Pointer into GFXfont->bitmap
    uint8_t  width;            ///< Bitmap dimensions in pixels
    uint8_t  height;           ///< Bitmap dimensions in pixels
    uint8_t  xAdvance;         ///< Distance to advance cursor (x axis)
    int8_t   xOffset;          ///< X dist from cursor pos to UL corner
    int8_t   yOffset;          ///< Y dist from cursor pos to UL corner
} GFXglyph;

typedef struct {
    uint8_t  *bitmap;      ///< Glyph bitmaps, concatenated
    GFXglyph *glyph;       ///< Glyph array
    uint16_t  first;       ///< ASCII extents (first char)
    uint16_t  last;        ///< ASCII extents (last char)
    uint8_t   yAdvance;    ///< Newline distance (y axis)
} GFXfont;

extern const GFXfont FreeSans9pt7b;

struct MockAdafruitPrintCall {
    std::string str;
    int16_t x;
    int16_t y;
    uint16_t color;
    const GFXfont* font;
};

struct MockAdafruitDrawLineCall {
    int16_t x0, y0, x1, y1;
    uint16_t color;
};

struct MockAdafruitFillRectCall {
    int16_t x, y, w, h;
    uint16_t color;
};

extern std::vector<MockAdafruitPrintCall> mockAdafruitPrintCalls;
extern std::vector<MockAdafruitDrawLineCall> mockAdafruitDrawLineCalls;
extern std::vector<MockAdafruitFillRectCall> mockAdafruitFillRectCalls;
extern int mockAdafruitInitCount;
extern int mockAdafruitSetRotationCount;
extern int mockAdafruitFillScreenCount;

class Adafruit_GFX {
public:
    Adafruit_GFX(int16_t w, int16_t h) : WIDTH(w), HEIGHT(h), _width(w), _height(h), cursor_x(0), cursor_y(0), textcolor(0xFFFF), gfxFont(nullptr) {}

    void setCursor(int16_t x, int16_t y) {
        cursor_x = x;
        cursor_y = y;
    }

    void setTextColor(uint16_t c) {
        textcolor = c;
    }

    void setFont(const GFXfont *f = nullptr) {
        gfxFont = f;
    }

    void print(const char* str) {
        mockAdafruitPrintCalls.push_back({str, cursor_x, cursor_y, textcolor, gfxFont});
    }

    void getTextBounds(const char *string, int16_t x, int16_t y,
                       int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
        // Dummy implementation that assumes 10px per char width, 15px height
        *x1 = x;
        *y1 = y;
        *w = std::string(string).length() * 10;
        *h = 15;
    }

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
        mockAdafruitDrawLineCalls.push_back({x0, y0, x1, y1, color});
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        mockAdafruitFillRectCalls.push_back({x, y, w, h, color});
    }

    void fillScreen(uint16_t color) {
        mockAdafruitFillScreenCount++;
        fillRect(0, 0, _width, _height, color);
    }

    void setRotation(uint8_t r) {
        mockAdafruitSetRotationCount++;
    }

    int16_t width() const { return _width; }
    int16_t height() const { return _height; }

protected:
    int16_t WIDTH, HEIGHT;
    int16_t _width, _height;
    int16_t cursor_x, cursor_y;
    uint16_t textcolor;
    const GFXfont *gfxFont;
};

#endif
