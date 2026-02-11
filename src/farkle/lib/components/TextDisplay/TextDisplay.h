#ifndef TextDisplay_h
#define TextDisplay_h

#include <Arduino.h>
#include <U8g2lib.h> // Include U8g2 library

// Define fonts as macros for easy configuration
#define TEXT_DISPLAY_MAIN_FONT u8g2_font_ncenB10_tr
#define TEXT_DISPLAY_TITLE_FONT u8g2_font_ncenB08_tr
#define TEXT_DISPLAY_LARGE_FONT u8g2_font_ncenB18_tr
#define TEXT_DISPLAY_MAIN_HEIGHT 15
#define TEXT_DISPLAY_TITLE_HEIGHT 12
#define TEXT_DISPLAY_LARGE_HEIGHT 24

// Define arrow geometry
#define TEXT_DISPLAY_ARROW_WIDTH 10
#define TEXT_DISPLAY_ARROW_HEIGHT 4
#define TEXT_DISPLAY_ARROW_SPACING 2 // Vertical spacing from text

// Buffer size for text caching
#define TEXT_DISPLAY_BUFFER_SIZE 64

enum class DisplayMode {
    NONE,
    MESSAGE,
    SELECTION,
    TITLE,
    TITLE_SUBTITLE,
    TITLE_SUBTITLES,
    CHARACTER_INPUT
};

class TextDisplay
{
  public:
    TextDisplay();
    void begin();

    // API from COMPONENT_LIBRARIES.md
    void print(const char* message);
    void printSelectionScreen(const char* selectionTitle, const char* selectionItem);
    void displayTitle(const char* title);
    void displayTitleWithSubtitle(const char* title, const char* subtitle);
    void displayTitleWithSubtitles(const char* title, const char* leftSubtitle, const char* rightSubtitle);
    void displayCharacterInput(const char* currentName, int activeIndex);

  private:
    U8G2_SH1106_128X64_NONAME_1_HW_I2C _display;

    // State tracking
    DisplayMode _currentMode;
    char _lastMessage[TEXT_DISPLAY_BUFFER_SIZE];
    char _lastTitle[TEXT_DISPLAY_BUFFER_SIZE];
    char _lastItem[TEXT_DISPLAY_BUFFER_SIZE];
    char _lastSubtitle[TEXT_DISPLAY_BUFFER_SIZE];
    char _lastLeftSubtitle[TEXT_DISPLAY_BUFFER_SIZE];
    char _lastRightSubtitle[TEXT_DISPLAY_BUFFER_SIZE];
    int _lastActiveIndex;

    // Helper to draw arrow
    void drawArrow(int x, int y, bool up);

    // Internal helper to update state and check if redraw is needed
    bool shouldRedraw(DisplayMode mode, const char* s1 = nullptr, const char* s2 = nullptr, const char* s3 = nullptr, int index = -1);
};

#endif
