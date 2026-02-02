#ifndef LedProgressGrid_h
#define LedProgressGrid_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <vector> // Required for std::vector

class LedProgressGrid {
public:
  LedProgressGrid(uint8_t pin);
  void begin();
  int addPlayer();
  void reset();
  void clear();
  void update(const std::vector<int>& scores, int currentPlayerIndex, int atRiskScore);

private:
  Adafruit_NeoPixel _pixels;
  uint16_t _num_pixels;

  int _playerCount;
  std::vector<uint16_t> _playerHues;
  int _maxScore;
  bool _isBlinkOn;

  void illuminate_row(int row, uint16_t hue, float ratio, uint8_t brightness = 255);
  int get_pixel_index(int row, int col);
  int getRemainderBrightness(float remainder, int fullBrightness);
};

#endif