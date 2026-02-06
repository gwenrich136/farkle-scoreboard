#include "LedProgressGrid.h"

#define GRID_LENGTH 8
#define GOLDEN_RATIO_CONJUGATE 0.61803398875f
#define BLINK_HALF_PERIOD 500

// Constructor: initializes the NeoPixel object
LedProgressGrid::LedProgressGrid(uint8_t pin)
  : _pixels(GRID_LENGTH*GRID_LENGTH, pin, NEO_GRB + NEO_KHZ800),
    _targetScore(10000),
    _hasProspectiveFirstHue(false)
{
    _lastState.isDirty = true;
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
  // We use a 4th degree polynomial to map the linear remainder (0..1) to a
  // brightness scale (0..1) that is more perceptually distinct.
  //
  // The polynomial was chosen with the following properties:
  //   - y(0) = 0
  //   - y(1) = 1
  //   - y'(0) = 0.25 (Slope at start)
  //   - y'(1) = 4.0  (Slope at end)
  //
  // This gives us a curve that increases slowly at first (allowing distinguishing
  // low values) and ramps up significantly at the end.
  //
  // The polynomial is:
  // y(x) = 2.7x^4 - 3.15x^3 + 1.2x^2 + 0.25x

  float x = remainder;
  float x2 = x * x;
  float x3 = x2 * x;
  float x4 = x3 * x;

  float y = (2.7f * x4) - (3.15f * x3) + (1.2f * x2) + (0.25f * x);

  return (int) (fullBrightness * y);
}

int LedProgressGrid::addPlayer() {
    uint16_t newHue = getPlayerHue(_playerCount);
    _playerHues.push_back(newHue);
    
    int playerIndex = _playerCount;
    _playerCount++;
    _lastState.isDirty = true;
    _hasProspectiveFirstHue = false;
    
    return playerIndex;
}

void LedProgressGrid::clear() {
  _pixels.clear();
  _pixels.show();
  _lastState = State();
  _lastState.isDirty = true;
}

void LedProgressGrid::reset() {
  _playerCount = 0;
  _playerHues.clear();
  _hasProspectiveFirstHue = false;

  _maxScore = _targetScore;
  _isBlinkOn = false;

  clear();
}

LedProgressGrid::PlayerRows LedProgressGrid::getRowMapping(int totalPlayers, int playerIdx) {
    int startRow = 0;
    int numRows = 0;
    switch (totalPlayers) {
      case 1: startRow = 2; numRows = 4; break;
      case 2: startRow = (playerIdx == 0) ? 0 : 5; numRows = 3; break;
      case 3: startRow = (playerIdx == 0) ? 0 : (playerIdx == 1 ? 3 : 6); numRows = 2; break;
      case 4: startRow = playerIdx * 2; numRows = 2; break;
      default: startRow = playerIdx; numRows = 1; break;
    }
    return {startRow, numRows};
}

bool LedProgressGrid::shouldRefresh(const State& newState) {
    if (newState != _lastState) {
        _lastState = newState;
        _lastState.isDirty = false;
        return true;
    }
    return false;
}

uint16_t LedProgressGrid::getPlayerHue(int playerIdx) {
    if (playerIdx < (int)_playerHues.size()) {
        return _playerHues[playerIdx];
    }

    // Pending player logic
    if (_playerHues.empty()) {
        if (!_hasProspectiveFirstHue) {
            _prospectiveFirstHue = random(0, 65536);
            _hasProspectiveFirstHue = true;
        }
        return _prospectiveFirstHue;
    } else {
        return (uint16_t)(_playerHues.back() + (GOLDEN_RATIO_CONJUGATE * 65536)) % 65536;
    }
}

void LedProgressGrid::renderPlayerRows(PlayerRows rows, uint16_t hue, float ratio, uint8_t brightness) {
    for (int r = 0; r < rows.numRows; ++r) {
        illuminate_row(rows.startRow + r, hue, ratio, brightness);
    }
}

void LedProgressGrid::update(const std::vector<int>& scores, int currentPlayerIndex, int atRiskScore) {
  _isBlinkOn = millis() % (2 * BLINK_HALF_PERIOD) > BLINK_HALF_PERIOD;

  State currentState;
  currentState.mode = DisplayMode::IN_GAME;
  currentState.scores = scores;
  currentState.currentPlayerIndex = currentPlayerIndex;
  currentState.atRiskScore = atRiskScore;
  currentState.isBlinkOn = _isBlinkOn;
  currentState.playerCount = _playerCount;
  currentState.isDirty = false;

  if (!shouldRefresh(currentState)) {
    return;
  }

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
    // As per design: "lowest multiple of 2000 greater than the highest score, with a minimum of _targetScore"
    int newMax = ( (highestScore / 2000) + 1) * 2000;
    _maxScore = max(_targetScore, newMax);
  }

  _pixels.clear();

  for (int playerIdx = 0; playerIdx < _playerCount; ++playerIdx) {
    PlayerRows rows = getRowMapping(_playerCount, playerIdx);
    
    int scoreToDraw = scores[playerIdx];
    bool showingRisk = (playerIdx == currentPlayerIndex && atRiskScore > 0 && _isBlinkOn);
    if (showingRisk) {
      scoreToDraw += atRiskScore;
    }

    float ratioToDraw = (float)scoreToDraw / _maxScore;
    if (ratioToDraw > 1.0f) ratioToDraw = 1.0f;

    renderPlayerRows(rows, _playerHues[playerIdx], ratioToDraw, 128);
  }

  _pixels.show();
}

void LedProgressGrid::displayPlayersPregame(bool isPlayerPending) {
  _isBlinkOn = millis() % (2 * BLINK_HALF_PERIOD) > BLINK_HALF_PERIOD;

  State currentState;
  currentState.mode = DisplayMode::PRE_GAME;
  currentState.isPlayerPending = isPlayerPending;
  currentState.isBlinkOn = _isBlinkOn;
  currentState.playerCount = _playerCount;
  currentState.isDirty = false;

  if (!shouldRefresh(currentState)) {
    return;
  }

  _pixels.clear();

  int effectivePlayerCount = _playerCount + (isPlayerPending ? 1 : 0);

  // Draw existing players
  for (int playerIdx = 0; playerIdx < _playerCount; ++playerIdx) {
      PlayerRows rows = getRowMapping(effectivePlayerCount, playerIdx);
      renderPlayerRows(rows, _playerHues[playerIdx], 1.0f, 128);
  }

  // Draw pending player
  if (isPlayerPending && _isBlinkOn) {
      int pendingIdx = _playerCount;
      PlayerRows rows = getRowMapping(effectivePlayerCount, pendingIdx);
      uint16_t hue = getPlayerHue(pendingIdx);
      renderPlayerRows(rows, hue, 1.0f, 128);
  }

  _pixels.show();
}
