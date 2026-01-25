#ifndef LedProgressGrid_h
#define LedProgressGrid_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LedProgressGrid {
public:
  LedProgressGrid(uint8_t pin, uint16_t num_pixels);
  void hello_world();

private:
  Adafruit_NeoPixel _pixels;
  uint16_t _num_pixels;
  void illuminate_row(int row, uint16_t hue, float ratio);
  int get_pixel_index(int row, int col);
};

#endif