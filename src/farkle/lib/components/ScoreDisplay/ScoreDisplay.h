#ifndef ScoreDisplay_h
#define ScoreDisplay_h

/**
 * ScoreDisplay
 *
 * This component manages three 5-digit 7-segment displays driven by MAX7219 chips.
 * It provides a high-level API for printing numbers and clearing displays, while
 * internally tracking the state of each display to optimize hardware communication.
 *
 * Responsibilities:
 * - Initialize and configure MAX7219 display drivers.
 * - Map logical display types (At Risk, Current Player, Competition) to physical devices.
 * - Format and render integers on the 5-digit displays with right-alignment.
 * - Handle score overflow by capping values at 99,999.
 * - Implement non-blocking blinking effects by toggling intensity.
 * - Maintain an internal cache (State) of the last rendered values to skip redundant updates.
 */

#include <Arduino.h>
#include <SPI.h>
#include <MD_MAX72xx.h>

#define NUM_DISPLAY_TYPES 3

class ScoreDisplay {
public:
  enum class DisplayType { AT_RISK_SCORE, CURRENT_PLAYER_SCORE, COMPETITION_SCORE };

  // Hardware SPI uses predefined MOSI, SCK, so we only need csPin
  ScoreDisplay(int csPin);
  void begin();
  void addDisplay(DisplayType type, int deviceIndex);
  void print_number(int number, DisplayType type, bool blink = false);
  void clear(DisplayType type);

  struct State {
    int number = -1;
    bool blink = false;
    bool isCleared = true;
    int lastIntensity = -1;

    bool operator==(const State& other) const {
      return number == other.number &&
             blink == other.blink &&
             isCleared == other.isCleared &&
             lastIntensity == other.lastIntensity;
    }
    bool operator!=(const State& other) const { return !(*this == other); }
  };

private:
  MD_MAX72XX _lc;
  int _deviceMap[NUM_DISPLAY_TYPES]; // Map DisplayType to deviceIndex
  State _states[NUM_DISPLAY_TYPES];

  bool isValidType(DisplayType type);
  void setState(DisplayType type, int number, bool blink, bool isCleared, int lastIntensity);
};

#endif
