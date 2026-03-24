#include "LedProgressGrid.h"
#include <cstring>

#define GRID_LENGTH 8
#define GOLDEN_RATIO_CONJUGATE 0.61803398875f
#define BLINK_HALF_PERIOD 500

// Constructor: initializes the NeoPixel object
LedProgressGrid::LedProgressGrid(uint8_t pin)
  : _pixels(GRID_LENGTH*GRID_LENGTH, pin, NEO_GRB + NEO_KHZ800),
    _targetScore(10000)
{
    memset(_playerHues, 0, sizeof(_playerHues));
    _lastState.isDirty = true;
}

void LedProgressGrid::begin() {
  _pixels.begin();           // Initialize NeoPixel strip object
  reset(); // Call reset to initialize all state and clear display
}

void LedProgressGrid::setTargetScore(int target) {
    _targetScore = target;
    _maxScore = target;
    _lastState.isDirty = true;
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

int LedProgressGrid::addPlayer(uint16_t hue) {
    if (isMaxPlayersReached()) {
        return -1;
    }

    _playerHues[_playerCount] = hue;
    
    int playerIndex = _playerCount;
    _playerCount++;
    _lastState.isDirty = true;
    
    return playerIndex;
}

bool LedProgressGrid::isMaxPlayersReached() {
    return _playerCount >= MAX_PLAYERS;
}

void LedProgressGrid::clear() {
  _pixels.clear();
  _pixels.show();
  _lastState = State();
  _lastState.isDirty = true;
}

void LedProgressGrid::reset() {
  _playerCount = 0;

  _maxScore = _targetScore;
  _isBlinkOn = false;

  clear();
}

bool LedProgressGrid::shouldRefresh(const State& newState) {
    if (newState != _lastState) {
        _lastState = newState;
        _lastState.isDirty = false;
        return true;
    }
    return false;
}

void LedProgressGrid::renderPlayerRows(PlayerRows rows, uint16_t hue, float ratio, uint8_t brightness) {
    for (int r = 0; r < rows.numRows; ++r) {
        illuminate_row(rows.startRow + r, hue, ratio, brightness);
    }
}

void LedProgressGrid::update(const int* scores, int playerCount, int currentPlayerIndex, int blinkingScore) {
  if (playerCount < _playerCount) {
    return;
  }

  _isBlinkOn = millis() % (2 * BLINK_HALF_PERIOD) > BLINK_HALF_PERIOD;

  // 1. Calculate new max score
  int highestScore = 0;
  for (int i = 0; i < _playerCount; ++i) {
    int potentialScore = scores[i];
    if (i == currentPlayerIndex) {
        potentialScore += blinkingScore;
    }
    if (potentialScore > highestScore) {
      highestScore = potentialScore;
    }
  }
  _maxScore = max(_targetScore, highestScore);

  // 2. Prepare state for refresh check
  State currentState;
  currentState.mode = DisplayMode::IN_GAME;

  // Assume _playerCount <= MAX_PLAYERS because addPlayer checks this.
  for (int i = 0; i < _playerCount; ++i) {
    currentState.scores[i] = scores[i];
  }
  // Zero out the rest of the array just to be safe/clean, though initialization did this.
  for (int i = _playerCount; i < MAX_PLAYERS; ++i) {
    currentState.scores[i] = 0;
  }

  currentState.currentPlayerIndex = currentPlayerIndex;
  currentState.blinkingScore = blinkingScore;
  currentState.isBlinkOn = _isBlinkOn;
  currentState.playerCount = _playerCount;
  currentState.maxScore = _maxScore;
  currentState.isDirty = false;

  if (!shouldRefresh(currentState)) {
    return;
  }

  _pixels.clear();

  for (int playerIdx = 0; playerIdx < _playerCount; ++playerIdx) {
    PlayerRows rows = PlayerLayout::getMapping(_playerCount, playerIdx);
    
    int scoreToDraw = scores[playerIdx];
    bool showingBlink = (playerIdx == currentPlayerIndex && blinkingScore > 0 && _isBlinkOn);
    if (showingBlink) {
      scoreToDraw += blinkingScore;
    }

    float ratioToDraw = (float)scoreToDraw / _maxScore;
    if (ratioToDraw > 1.0f) ratioToDraw = 1.0f;

    renderPlayerRows(rows, _playerHues[playerIdx], ratioToDraw, 128);
  }

  _pixels.show();
}

void LedProgressGrid::displayPlayersPregame(std::optional<uint16_t> pendingPlayerHue) {
  bool isPlayerPending = pendingPlayerHue.has_value();
  if (isMaxPlayersReached()) {
    isPlayerPending = false;
  }

  _isBlinkOn = millis() % (2 * BLINK_HALF_PERIOD) > BLINK_HALF_PERIOD;

  State currentState;
  currentState.mode = DisplayMode::PRE_GAME;
  currentState.isPlayerPending = isPlayerPending;
  currentState.isBlinkOn = _isBlinkOn;
  currentState.playerCount = _playerCount;
  currentState.maxScore = _maxScore;
  currentState.isDirty = false;

  if (!shouldRefresh(currentState)) {
    return;
  }

  _pixels.clear();

  int effectivePlayerCount = _playerCount + (isPlayerPending ? 1 : 0);

  // Draw existing players
  for (int playerIdx = 0; playerIdx < _playerCount; ++playerIdx) {
      PlayerRows rows = PlayerLayout::getMapping(effectivePlayerCount, playerIdx);
      renderPlayerRows(rows, _playerHues[playerIdx], 1.0f, 128);
  }

  // Draw pending player
  if (isPlayerPending && _isBlinkOn) {
      int pendingIdx = _playerCount;
      PlayerRows rows = PlayerLayout::getMapping(effectivePlayerCount, pendingIdx);
      renderPlayerRows(rows, *pendingPlayerHue, 1.0f, 128);
  }

  _pixels.show();
}
