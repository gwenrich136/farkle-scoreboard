#include "LedProgressGrid.h"

#define GRID_LENGTH 8

// Constructor: initializes the NeoPixel object
LedProgressGrid::LedProgressGrid(uint8_t pin, uint16_t num_pixels)
  : _pixels(num_pixels, pin, NEO_GRB + NEO_KHZ800), _num_pixels(num_pixels)
{
  // Constructor is empty, actual initialization is in begin()
  _pixels.begin();           // Initialize NeoPixel strip object (this loads the first pixel data to the NeoPixel's RAM, but does not actually display it yet)
  _pixels.show();            // Turn off all pixels (by showing the empty RAM data)
}

// hello_world method: cycles through pixels with random HSV colors
void LedProgressGrid::hello_world()
{
  int random_offset = random(0,65536);
  for (int i = 0; i < 8; ++i) {
    int hue = (int) (65536 * i * 1.618 + random_offset) % 65536;
    illuminate_row(i, hue, random(0,100)/100.0);
  }
  _pixels.show();
}

void LedProgressGrid::illuminate_row(int row, uint16_t hue, float ratio) {
  int num_pixels = (int) (ratio * GRID_LENGTH);
  float remainder = (ratio * GRID_LENGTH) - num_pixels;
  int remaininderBrightness = remainder * 256;
  // rows snake, so we need to count backwards for odd rows
    for (int col = 0; col < num_pixels; ++col) {
      _pixels.setPixelColor(
        get_pixel_index(row, col),
        _pixels.ColorHSV(hue, 255, 255));
    }
    _pixels.setPixelColor(
      get_pixel_index(row, num_pixels),
      _pixels.ColorHSV(hue, 255, remaininderBrightness)
    );

}

int LedProgressGrid::get_pixel_index(int row, int col) {
  return (row * GRID_LENGTH) + (row % 2 == 0 ? col : (GRID_LENGTH - 1) - col);
}