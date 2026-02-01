#ifndef ButtonActions_h
#define ButtonActions_h

// This enum represents all possible logical game actions triggered by the control pad.
// It is defined in a central location to decouple game logic from the ControlPad hardware library.
enum ButtonAction {
  NONE,
  BANK,
  FARKLE,
  CLEAR,
  DOWN_50,
  LEFT_100,
  RIGHT_500,
  UP_1000
};

#endif
