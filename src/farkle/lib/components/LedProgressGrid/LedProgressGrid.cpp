#include <cmath>
#include "LedProgressGrid.h"

#define GRID_LENGTH 8

// Constructor: initializes the NeoPixel object
LedProgressGrid::LedProgressGrid(uint8_t pin)
  : _pixels(GRID_LENGTH*GRID_LENGTH, pin, NEO_GRB + NEO_KHZ800)
{
}

void LedProgressGrid::begin() {
  _pixels.begin();           // Initialize NeoPixel strip object
  reset(); // Call reset to initialize all state and clear display
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

void LedProgressGrid::illuminate_row(int row, uint16_t hue, float ratio, uint8_t brightness) {
  int num_pixels = (int) (ratio * GRID_LENGTH);
  float remainder = (ratio * GRID_LENGTH) - num_pixels;
  // Apply a cubic polynomial for a more perceptually linear brightness curve.
  // This curve satisfies y(0)=0, y(1)=1, y'(0)=1/4, and y'(1)=4.
  float x = remainder;
  float y = (9.0/4.0) * x*x*x - (3.0/2.0) * x*x + (1.0/4.0) * x;
  int remainderBrightness = (int)(y * brightness);
  // rows snake, so we need to count backwards for odd rows
    for (int col = 0; col < num_pixels; ++col) {
      _pixels.setPixelColor(
        get_pixel_index(row, col),
        _pixels.ColorHSV(hue, 255, brightness));
    }
    // Only draw partial pixel if we haven't filled the row
    if (num_pixels < GRID_LENGTH) {
      _pixels.setPixelColor(
        get_pixel_index(row, num_pixels),
        _pixels.ColorHSV(hue, 255, remainderBrightness)
      );
    }
}

int LedProgressGrid::get_pixel_index(int row, int col) {
  return (row * GRID_LENGTH) + (row % 2 != 0 ? col : (GRID_LENGTH - 1) - col);
}

const float GOLDEN_RATIO_CONJUGATE = 0.61803398875;

int LedProgressGrid::addPlayer() {
    uint16_t newHue;
    if (_playerHues.empty()) {
        // First player gets a random hue
        newHue = random(0, 65536);
    } else {
        // Subsequent players get hues based on the golden ratio for good distribution
        uint16_t lastHue = _playerHues.back();
        newHue = (uint16_t)(lastHue + (GOLDEN_RATIO_CONJUGATE * 65536)) % 65536;
    }
    _playerHues.push_back(newHue);
    
    int playerIndex = _playerCount;
    _playerCount++;
    
    return playerIndex;
}

void LedProgressGrid::clear() {
  _pixels.clear();
  _pixels.show();
}

void LedProgressGrid::reset() {
  // Reset player configurations
  _playerCount = 0;
  _playerHues.clear();

  // Reset score scaling
  _maxScore = 10000;
  _isBlinkOn = false;

  // Turn off all LEDs
  clear();
}

void LedProgressGrid::update(const std::vector<int>& scores, int currentPlayerIndex, int atRiskScore) {
  // 1. Update blink state
  _isBlinkOn = millis() % 800 > 400;

  // 2. Update max score
  int highestScore = 0;
  for (int i = 0; i < _playerCount; ++i) {
    int potentialScore = scores[i];
    if (i == currentPlayerIndex) {
        potentialScore += atRiskScore;
    }
    if (potentialScore > highestScore) {
      highestScore = potentialScore;
    }
  }
  
  if (highestScore > _maxScore) {
    // As per design: "lowest multiple of 2000 greater than the highest score, with a minimum of 10,000"
    int newMax = ( (highestScore / 2000) + 1) * 2000;
    _maxScore = max(10000, newMax);
  }

  // 3. Clear the buffer before drawing
  _pixels.clear();

  // 4. Loop through players and draw their bars
  for (int playerIdx = 0; playerIdx < _playerCount; ++playerIdx) {
    int startRow = 0;
    int numRows = 0;

    // --- Get Player Row Mapping (as per design) ---
    switch (_playerCount) {
      case 1: startRow = 2; numRows = 4; break;
      case 2: startRow = (playerIdx == 0) ? 0 : 5; numRows = 3; break;
      case 3: startRow = (playerIdx == 0) ? 0 : (playerIdx == 1 ? 3 : 6); numRows = 2; break;
      case 4: startRow = playerIdx * 2; numRows = 2; break;
      default: startRow = playerIdx; numRows = 1; break; // 5-8 players
    }
    
    // --- Calculate ratios ---
    // Total Ratio (Banked + Risk)
    int totalScore = scores[playerIdx];
    if (playerIdx == currentPlayerIndex) {
      totalScore += atRiskScore;
    }
    float totalRatio = (float)totalScore / _maxScore;
    if (totalRatio > 1.0f) totalRatio = 1.0f;

    // Banked Ratio (Solid part)
    float bankedRatio = (float)scores[playerIdx] / _maxScore;
    if (bankedRatio > 1.0f) bankedRatio = 1.0f;

    // --- Draw each row ---
    for (int r = 0; r < numRows; ++r) {
      int physicalRow = startRow + r;
      
      bool showingRisk = (playerIdx == currentPlayerIndex && atRiskScore > 0 && _isBlinkOn);

      // Pass 1: Draw At-Risk (Dim Extension) - only for current player if blinking
      if (showingRisk) {
         // Draw the full bar (banked + risk) at half brightness (128).
         // Do NOT clear excess, because global clear handled it, and we are about to draw banked over it anyway.
         illuminate_row(physicalRow, _playerHues[playerIdx], totalRatio, 128);
      } else {
        illuminate_row(physicalRow, _playerHues[playerIdx], bankedRatio, 128);
      }
    }
  }

  // 5. Show the updated pixels
  _pixels.show();
}
