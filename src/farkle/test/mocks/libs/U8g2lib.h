#ifndef U8g2lib_h
#define U8g2lib_h

#include <stdint.h>
#include <string>
#include <vector>

// Types
typedef void (*u8g2_cb_t)(void);
typedef int u8g2_uint_t;

// Constants
#define U8X8_PIN_NONE 255
extern const uint8_t u8g2_font_ncenB10_tr[];
extern const uint8_t u8g2_font_ncenB08_tr[]; // For smaller font if needed
extern const uint8_t u8g2_font_ncenB18_tr[]; // For large font

// Rotation callback mock
extern void u8g2_cb_r0(void);
#define U8G2_R0 u8g2_cb_r0

// Mock tracking structures
struct MockDrawStrCall {
    int x;
    int y;
    std::string str;
};

struct MockDrawLineCall {
    int x1;
    int y1;
    int x2;
    int y2;
};

// Global tracking variables
extern int mockU8g2BeginCount;
extern int mockU8g2SetFontCount;
extern std::vector<MockDrawStrCall> mockU8g2DrawStrCalls;
extern std::vector<MockDrawLineCall> mockU8g2DrawLineCalls;
extern std::string mockU8g2LastFont;

class U8G2_SH1106_128X64_NONAME_1_HW_I2C {
public:
    U8G2_SH1106_128X64_NONAME_1_HW_I2C(u8g2_cb_t rotation, uint8_t reset = U8X8_PIN_NONE, uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE) {}

    void setI2CAddress(uint8_t adr) {}

    uint8_t begin() {
        mockU8g2BeginCount++;
        return 1;
    }

    void setFont(const uint8_t *font) {
        mockU8g2SetFontCount++;
        if (font == u8g2_font_ncenB10_tr) mockU8g2LastFont = "ncenB10";
        else if (font == u8g2_font_ncenB08_tr) mockU8g2LastFont = "ncenB08";
        else if (font == u8g2_font_ncenB18_tr) mockU8g2LastFont = "ncenB18";
        else mockU8g2LastFont = "unknown";
    }

    void setFontPosTop() {}
    void setFontRefHeightExtendedText() {}

    int getStrWidth(const char *s) {
        return std::string(s).length() * 10; // Simplified width calculation
    }

    int getDisplayWidth() { return 128; }
    int getDisplayHeight() { return 64; }

    void firstPage() { _pageCount = 0; }
    uint8_t nextPage() { 
        _pageCount++;
        return (_pageCount < 2) ? 1 : 0; // Return 1 once to allow one loop iteration
    }

    int drawStr(int x, int y, const char *s) {
        mockU8g2DrawStrCalls.push_back({x, y, s});
        return 0;
    }

    void drawLine(int x1, int y1, int x2, int y2) {
        mockU8g2DrawLineCalls.push_back({x1, y1, x2, y2});
    }

    // Additional methods if needed
    void sendBuffer() {}
    void clearBuffer() {}
private:
    int _pageCount = 0;
};

#endif
