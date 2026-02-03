#include "LedProgressGrid.h"

#define GRID_LENGTH 8
#define GOLDEN_RATIO_CONJUGATE 0.61803398875f

// Constructor: initializes the NeoPixel object
LedProgressGrid::LedProgressGrid(uint8_t pin)
  : _pixels(GRID_LENGTH*GRID_LENGTH, pin, NEO_GRB + NEO_KHZ800),
    _hasProspectiveFirstHue(false),
    _isDirty(true)
{
}

void LedProgressGrid::begin() {
  _pixels.begin();           // Initialize NeoPixel strip object
  reset(); // Call reset to initialize all state and clear display
}

void LedProgressGrid::illuminate_row(int row, uint16_t hue, float ratio, uint8_t brightness) {
  int num_pixels = (int) (ratio * GRID_LENGTH);
  float remainder = (ratio * GRID_LENGTH) - num_pixels;
  // rows snake, so we need to count backwards for odd rows
    uint32_t color = _pixels.ColorHSV(hue, 255, brightness);
    for (int col = 0; col < num_pixels; ++col) {
      _pixels.setPixelColor(
        get_pixel_index(row, col),
        color);
    }
    // Only draw partial pixel if we haven't filled the row
    if (num_pixels < GRID_LENGTH) {
      _pixels.setPixelColor(
        get_pixel_index(row, num_pixels),
        _pixels.ColorHSV(hue, 255, getRemainderBrightness(remainder, brightness))
      );
    }
}

int LedProgressGrid::get_pixel_index(int row, int col) {
  return (row * GRID_LENGTH) + (row % 2 != 0 ? col : (GRID_LENGTH - 1) - col);
}

int LedProgressGrid::getRemainderBrightness(float remainder, int fullBrightness) {
  // The following is a cubic expression in terms of it's derrivate at 
  // x=0 and x=1, denoted by y'(0) and y'(1), with the following constraints
  //   - y(0) = 0
  //   - y(1) = 1
  // 
  // y(x) = (y'(1) - y'(0) - 2) * x^3 
  //        + (3 - y'(1) - 2 * y'(0)) * x^2 
  //        + y'(0) * x
  //
  // With a desired y'(0) of 1/3 and y'(1) of 3, we get:
  //   - y(x) = (4/3) x^3 - (2/3) x^2 + (1/3) x
  float x = remainder;
  float y = (4*x*x - 2*x + 1) * x / 3.0;
  return (int) fullBrightness * y;
}

int LedProgressGrid::addPlayer() {
    uint16_t newHue;
    if (_playerHues.empty()) {
        // First player gets a random hue
        if (_hasProspectiveFirstHue) {
            newHue = _prospectiveFirstHue;
        } else {
            newHue = random(0, 65536);
        }
    } else {
        // Subsequent players get hues based on the golden ratio for good distribution
        uint16_t lastHue = _playerHues.back();
        newHue = (uint16_t)(lastHue + (GOLDEN_RATIO_CONJUGATE * 65536)) % 65536;
    }
    _playerHues.push_back(newHue);
    
    int playerIndex = _playerCount;
    _playerCount++;
    _isDirty = true;
    _hasProspectiveFirstHue = false; // Reset for next potential pregame call
    
    return playerIndex;
}

void LedProgressGrid::clear() {
  _pixels.clear();
  _pixels.show();
  _isDirty = true;
  _lastState = State(); // Reset memory
}

void LedProgressGrid::reset() {
  // Reset player configurations
  _playerCount = 0;
  _playerHues.clear();
  _hasProspectiveFirstHue = false;

  // Reset score scaling
  _maxScore = 10000;
  _isBlinkOn = false;
  _isDirty = true;
  _lastState = State(); // Reset memory

  // Turn off all LEDs
  clear();
}

void LedProgressGrid::update(const std::vector<int>& scores, int currentPlayerIndex, int atRiskScore) {
  // 1. Update blink state
  _isBlinkOn = millis() % 1000 > 500;

  // 1.5. Check Memory
  State currentState;
  currentState.mode = DisplayMode::IN_GAME;
  currentState.scores = scores;
  currentState.currentPlayerIndex = currentPlayerIndex;
  currentState.atRiskScore = atRiskScore;
  currentState.isBlinkOn = _isBlinkOn;
  currentState.playerCount = _playerCount;

  if (!_isDirty && currentState == _lastState) {
    return;
  }
  _isDirty = false;
  _lastState = currentState;

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
         illuminate_row(physicalRow, _playerHues[playerIdx], totalRatio, 128);
      } else {
        illuminate_row(physicalRow, _playerHues[playerIdx], bankedRatio, 128);
      }
    }
  }

  // 5. Show the updated pixels
  _pixels.show();
}

void LedProgressGrid::displayPlayersPregame(bool isPlayerPending) {
  // 1. Update blink state
  _isBlinkOn = millis() % 1000 > 500;

  // 2. Check Memory
  State currentState;
  currentState.mode = DisplayMode::PRE_GAME;
  currentState.isPlayerPending = isPlayerPending;
  currentState.isBlinkOn = _isBlinkOn;
  currentState.playerCount = _playerCount;

  if (!_isDirty && currentState == _lastState) {
    return;
  }
  _isDirty = false;
  _lastState = currentState;

  // 3. Clear the buffer before drawing
  _pixels.clear();

  // 4. Draw existing players
  for (int playerIdx = 0; playerIdx < _playerCount; ++playerIdx) {
      int startRow = 0;
      int numRows = 0;
      switch (_playerCount + (isPlayerPending ? 1 : 0)) {
          case 1: startRow = 2; numRows = 4; break;
          case 2: startRow = (playerIdx == 0) ? 0 : 5; numRows = 3; break;
          case 3: startRow = (playerIdx == 0) ? 0 : (playerIdx == 1 ? 3 : 6); numRows = 2; break;
          case 4: startRow = playerIdx * 2; numRows = 2; break;
          default: startRow = playerIdx; numRows = 1; break;
      }
      for (int r = 0; r < numRows; ++r) {
          illuminate_row(startRow + r, _playerHues[playerIdx], 1.0f, 128);
      }
  }

  // 5. Draw pending player if applicable
  if (isPlayerPending) {
      int pendingPlayerIdx = _playerCount;
      int startRow = 0;
      int numRows = 0;
      int totalPlayersIfAdded = _playerCount + 1;
      switch (totalPlayersIfAdded) {
          case 1: startRow = 2; numRows = 4; break;
          case 2: startRow = (pendingPlayerIdx == 0) ? 0 : 5; numRows = 3; break;
          case 3: startRow = (pendingPlayerIdx == 0) ? 0 : (pendingPlayerIdx == 1 ? 3 : 6); numRows = 2; break;
          case 4: startRow = pendingPlayerIdx * 2; numRows = 2; break;
          default: startRow = pendingPlayerIdx; numRows = 1; break;
      }

      if (_isBlinkOn) {
          uint16_t prospectiveHue;
          if (_playerHues.empty()) {
              if (!_hasProspectiveFirstHue) {
                  _prospectiveFirstHue = random(0, 65536);
                  _hasProspectiveFirstHue = true;
              }
              prospectiveHue = _prospectiveFirstHue;
          } else {
              prospectiveHue = (uint16_t)(_playerHues.back() + (GOLDEN_RATIO_CONJUGATE * 65536)) % 65536;
          }
          for (int r = 0; r < numRows; ++r) {
              illuminate_row(startRow + r, prospectiveHue, 1.0f, 128);
          }
      }
  }

  // 6. Show the updated pixels
  _pixels.show();
}
