#ifndef Input_h
#define Input_h

enum class ButtonAction {
    NONE,
    BANK,
    FARKLE,
    SELECT,
    CLEAR,
    PLUS_50,
    PLUS_100,
    PLUS_500,
    UNDO
};

enum class ScoreDisplayMode {
    BANKED,
    PENDING
};

struct GameInput {
    ButtonAction action;
    int rotationDelta;
    ScoreDisplayMode scoreDisplayMode;
};

#endif
