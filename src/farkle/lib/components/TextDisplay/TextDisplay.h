#ifndef TextDisplay_h
#define TextDisplay_h

#include <Arduino.h>
#include <U8g2lib.h> // Include U8g2 library

class TextDisplay
{
  public:
    TextDisplay();
    void print(const char* message);
    void begin();
  private:
    U8G2_SH1106_128X64_NONAME_F_HW_I2C _display;
};

#endif
