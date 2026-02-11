#ifndef TextDisplay_h
#define TextDisplay_h

#include <Arduino.h>
#include <U8g2lib.h> // Include U8g2 library
#include <string>

// Define fonts as macros for easy configuration
#define TEXT_DISPLAY_MAIN_FONT u8g2_font_ncenB10_tr
#define TEXT_DISPLAY_TITLE_FONT u8g2_font_ncenB08_tr
#define TEXT_DISPLAY_MAIN_HEIGHT 15
#define TEXT_DISPLAY_TITLE_HEIGHT 12

// Define arrow geometry
#define TEXT_DISPLAY_ARROW_SIZE 8 // Height/Width of arrow in pixels
#define TEXT_DISPLAY_ARROW_SPACING 6 // Vertical spacing from text

enum class DisplayMode {
    NONE,
    MESSAGE,
    SELECTION
};

class TextDisplay
{
  public:
    TextDisplay();
    void print(const char* message);
    void printSelectionScreen(const char* selectionTitle, const char* selectionItem);
    void begin();
  private:
    U8G2_SH1106_128X64_NONAME_1_HW_I2C _display;

    // State tracking
    DisplayMode _currentMode;
    std::string _lastMessage;
    std::string _lastTitle;
    std::string _lastItem;

    // Helper to draw arrow
    void drawArrow(int x, int y, bool up);
};

#endif
